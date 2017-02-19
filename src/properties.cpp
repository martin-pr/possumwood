#include "properties.h"

#include <QHeaderView>

Properties::Properties(QWidget* parent) : QTreeWidget(parent) {
	header()->hide();
	setRootIsDecorated(false);
}

void Properties::show(const std::vector<std::reference_wrapper<dependency_graph::Node>>& selection) {
	clear();

	for(auto& node : selection) {
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, node.get().name().c_str());

		addTopLevelItem(item);
	}
}
