#include "tree_widget.h"

#include <QHBoxLayout>

#include <dependency_graph/network.h>
#include <dependency_graph/node.h>

#include "adaptor.h"

TreeWidget::TreeWidget(QWidget* parent, Adaptor* adaptor) : QWidget(parent), m_adaptor(adaptor) {
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);

	m_tree = new QTreeWidget();
	m_tree->headerItem()->setHidden(true);
	m_tree->sortByColumn(0, Qt::AscendingOrder);
	m_tree->setSortingEnabled(true);
	layout->addWidget(m_tree);

	QTreeWidgetItem* root = new QTreeWidgetItem();
	root->setText(0, "/");
	root->setIcon(0, QIcon(":icons/network.png"));
	m_tree->addTopLevelItem(root);

	m_items.left.insert(std::make_pair(m_adaptor->graph().index(), root));

	m_signals.push_back(m_adaptor->graph().onAddNode(
		[this](dependency_graph::NodeBase& node) { onAddNode(node); }
	));

	m_signals.push_back(m_adaptor->graph().onRemoveNode(
		[this](dependency_graph::NodeBase& node) { onRemoveNode(node); }
	));

	connect(adaptor, &Adaptor::currentNetworkChanged, this, &TreeWidget::onCurrentNetworkChanged);
	connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &TreeWidget::onCurrentSelectionChanged);
}

TreeWidget::~TreeWidget() {
	for(auto& s : m_signals)
		s.disconnect();
}

void TreeWidget::onAddNode(dependency_graph::NodeBase& node) {
	assert(node.hasParentNetwork());

	auto parent = m_items.left.find(node.network().index());
	assert(parent != m_items.left.end());

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, node.name().c_str());

	if(node.is<dependency_graph::Network>())
		item->setIcon(0, QIcon(":icons/network.png"));
	else
		item->setIcon(0, QIcon(":icons/node.png"));
	parent->second->addChild(item);

	m_items.left.insert(std::make_pair(node.index(), item));
}

void TreeWidget::onRemoveNode(dependency_graph::NodeBase& node) {
	auto it = m_items.left.find(node.index());
	assert(it != m_items.left.end());

	if(it->second != nullptr && it->second->parent() != nullptr)
		it->second->parent()->removeChild(it->second);
	delete it->second;
	m_items.left.erase(it);
}

void TreeWidget::onCurrentNetworkChanged(const dependency_graph::NodeBase& node) {
	auto it = m_items.left.find(node.index());
	assert(it != m_items.left.end());

	if(!it->second->isExpanded())
		it->second->setExpanded(true);
}

void TreeWidget::onSelectionChanged(const dependency_graph::Selection& selection) {
	bool originalBlock = m_tree->blockSignals(true);

	m_tree->clearSelection();

	for(auto& n : selection.nodes()) {
		auto it = m_items.left.find(n.get().index());
		assert(it != m_items.left.end());

		it->second->setSelected(true);
	}

	m_tree->blockSignals(originalBlock);
}

void TreeWidget::onCurrentSelectionChanged() {
	// build a selection
	dependency_graph::Selection selection;
	for(auto& s : m_tree->selectedItems()) {
		auto it = m_items.right.find(s);
		assert(it != m_items.right.end());
		dependency_graph::UniqueId id = it->second;

		auto ii = m_adaptor->index().find(id);
		if(ii != m_adaptor->index().end()) {
			dependency_graph::NodeBase* node = ii->second.graphNode;
			if(node) {
				selection.addNode(*node);

				if(node->hasParentNetwork())
					for(auto& con : node->network().connections())
						if(con.first.node().index() == node->index() || con.second.node().index() == node->index())
							selection.addConnection(con.first, con.second);
			}
		}
	}

	// and set this selection in the parent adaptor
	bool originalBlock = m_tree->blockSignals(true);
	m_adaptor->setSelection(selection);
	m_tree->blockSignals(originalBlock);
}
