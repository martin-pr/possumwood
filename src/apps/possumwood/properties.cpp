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

	for(auto& node : selection) {
		// create a top level item for each node
		QTreeWidgetItem* nodeItem = new QTreeWidgetItem();
		nodeItem->setText(0, node.get().name().c_str());

		addTopLevelItem(nodeItem);
	}
}
