#include "tree_widget.h"

#include <QHBoxLayout>

#include <dependency_graph/network.h>

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

	m_items.insert(std::make_pair(m_adaptor->graph().index(), root));

	m_signals.push_back(m_adaptor->graph().onAddNode(
		[this](dependency_graph::NodeBase& node) { onAddNode(node); }
	));

	m_signals.push_back(m_adaptor->graph().onRemoveNode(
		[this](dependency_graph::NodeBase& node) { onRemoveNode(node); }
	));

	connect(adaptor, &Adaptor::currentNetworkChanged, this, &TreeWidget::onCurrentNetworkChanged);
}

TreeWidget::~TreeWidget() {
	for(auto& s : m_signals)
		s.disconnect();
}

void TreeWidget::onAddNode(dependency_graph::NodeBase& node) {
	assert(node.hasParentNetwork());

	auto parent = m_items.find(node.network().index());
	assert(parent != m_items.end());

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, node.name().c_str());

	if(node.is<dependency_graph::Network>())
		item->setIcon(0, QIcon(":icons/network.png"));
	else
		item->setIcon(0, QIcon(":icons/node.png"));
	parent->second->addChild(item);

	m_items.insert(std::make_pair(node.index(), item));
}

void TreeWidget::onRemoveNode(dependency_graph::NodeBase& node) {
	auto it = m_items.find(node.index());
	assert(it != m_items.end());

	it->second->parent()->removeChild(it->second);
	delete it->second;
	m_items.erase(it);
}

void TreeWidget::onCurrentNetworkChanged(const dependency_graph::NodeBase& node) {
	auto it = m_items.find(node.index());
	assert(it != m_items.end());

	if(!it->second->isExpanded())
		it->second->setExpanded(true);
}

