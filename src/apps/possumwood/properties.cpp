#include "properties.h"

#include <QHeaderView>

Properties::Properties(QWidget* parent) : QTreeWidget(parent) {
	setRootIsDecorated(false);

	headerItem()->setText(0, "item");
	headerItem()->setText(1, "value");
	headerItem()->setFirstColumnSpanned(false);
	header()->setStretchLastSection(true);
}

void Properties::show(const std::vector<std::reference_wrapper<dependency_graph::Node>>& selection) {
	clear();

	for(const auto& node : selection) {
		// create a top level item for each node
		QTreeWidgetItem* nodeItem = new QTreeWidgetItem();
		nodeItem->setText(0, node.get().name().c_str());

		addTopLevelItem(nodeItem);

		// add each port as a subitem
		for(unsigned pi = 0; pi != node.get().portCount(); ++pi) {
			const auto& port = node.get().port(pi);

			QTreeWidgetItem* portItem = new QTreeWidgetItem();
			portItem->setText(0, port.name().c_str());

			nodeItem->addChild(portItem);
		}
	}

	expandAll();
}
