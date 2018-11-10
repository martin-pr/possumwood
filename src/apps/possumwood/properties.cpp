#include "properties.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <QHeaderView>

#include <dependency_graph/node_base.inl>
#include <dependency_graph/port.inl>

#include <actions/actions.h>

Properties::Properties(QWidget* parent) : QTreeWidget(parent) {
	// setRootIsDecorated(false);

	headerItem()->setText(0, "item");
	headerItem()->setText(1, "value");
	headerItem()->setFirstColumnSpanned(false);
	header()->setStretchLastSection(true);

	connect(this, &Properties::itemChanged, [this](QTreeWidgetItem* item, int column) {
		auto it = m_nodes.find(item);
		assert(it != m_nodes.end());

		possumwood::actions::renameNode(*it->second, item->text(0).toStdString());
	});
}

void Properties::show(const dependency_graph::Selection& selection) {
	clear();
	m_properties.clear();
	m_nodes.clear();

	bool block = blockSignals(true);

	for(const auto& node : selection.nodes()) {
		// create a top level item for each node
		QTreeWidgetItem* nodeItem = new QTreeWidgetItem();
		nodeItem->setText(0, node.get().name().c_str());
		addTopLevelItem(nodeItem);
		nodeItem->setFirstColumnSpanned(true);
		nodeItem->setFlags(nodeItem->flags() | Qt::ItemIsEditable);

		m_nodes.insert(std::make_pair(nodeItem, &(node.get())));

		// add each port as a subitem
		std::map<std::string, QTreeWidgetItem*> parentItems;
		for(unsigned pi = 0; pi != node.get().portCount(); ++pi) {
			auto& port = node.get().port(pi);

			m_properties.push_back(PropertyHolder(port));

			if(m_properties.back().ui) {
				std::vector<std::string> pieces;
				boost::algorithm::split(pieces, port.name(), boost::is_any_of("/"));

				QTreeWidgetItem* currentItem = nodeItem;
				std::string currentString = "";

				while(!pieces.empty()) {
					currentString += "/" + *pieces.begin();

					auto it = parentItems.find(currentString);
					if(it == parentItems.end()) {
						QTreeWidgetItem* item = new QTreeWidgetItem();
						item->setText(0, pieces.begin()->c_str());
						item->setFirstColumnSpanned(true);
						currentItem->addChild(item);

						it = parentItems.insert(std::make_pair(currentString, item)).first;
					}

					currentItem = it->second;

					pieces.erase(pieces.begin());
				}

				currentItem->setFirstColumnSpanned(false);
				setItemWidget(currentItem, 1, m_properties.back().ui->widget());
			}
		}
	}

	blockSignals(block);

	expandAll();
	resizeColumnToContents(0);
}

////////

Properties::PropertyHolder::PropertyHolder(dependency_graph::Port& port) {
	// create the widget
	ui = possumwood::properties::factories::singleton().create(port.type());
	if(ui) {
		possumwood::properties::property_base* prop = ui.get();

		// change of port value
		graphValueConnection = port.valueCallback([prop, &port]() {
			prop->valueFromPort(port);
		});

		// change of port flags
		std::function<void()> updateFlags = [prop, &port]() {
			// build the flags based on port's properties
			unsigned flags = 0;

			if(port.category() == dependency_graph::Attr::kInput) {
				flags |= possumwood::properties::property_base::kInput;
				if(port.node().network().connections().connectedFrom(port))
					flags |= possumwood::properties::property_base::kDisabled;
			}

			if(port.category() == dependency_graph::Attr::kOutput)
				flags |= possumwood::properties::property_base::kOutput;

			// if(port.isDirty())
			// 	flags |= possumwood::properties::property_base::kDirty;

			// and set the flags on the property (calls UI's update, if implemented)
			prop->setFlags(flags);

			// immediate refresh when a port is dirty (pulls on the port)
			if(port.isDirty())
				prop->valueFromPort(port);
		};

		flagsConnection = port.flagsCallback(updateFlags);

		uiValueConnection = ui->valueCallback([prop, &port]() {
			// value changes only for non-disabled input port UIs
			// (avoid Qt's value changed callbacks when not necessary)
			if((prop->flags() & possumwood::properties::property_base::kInput) && !(prop->flags() & possumwood::properties::property_base::kDisabled))
				prop->valueToPort(port);
		});

		// run the update functions for the first time to set up the UI
		updateFlags();
		ui->valueFromPort(port);
	}
}

Properties::PropertyHolder::PropertyHolder(PropertyHolder&& prop) {
	ui = std::move(prop.ui);

	graphValueConnection = prop.graphValueConnection;
	prop.graphValueConnection = boost::signals2::connection();

	uiValueConnection = prop.uiValueConnection;
	prop.uiValueConnection = boost::signals2::connection();

	flagsConnection = prop.flagsConnection;
	prop.flagsConnection = boost::signals2::connection();
}

Properties::PropertyHolder::~PropertyHolder() {
	graphValueConnection.disconnect();
	uiValueConnection.disconnect();
	flagsConnection.disconnect();

}
