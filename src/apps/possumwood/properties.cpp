#include "properties.h"

#include <QHeaderView>

#include <dependency_graph/node.inl>
#include <dependency_graph/port.inl>

Properties::Properties(QWidget* parent) : QTreeWidget(parent) {
	// setRootIsDecorated(false);

	headerItem()->setText(0, "item");
	headerItem()->setText(1, "value");
	headerItem()->setFirstColumnSpanned(false);
	header()->setStretchLastSection(true);

	connect(this, &Properties::itemChanged, [this](QTreeWidgetItem* item, int column) {
		auto it = m_nodes.find(item);
		assert(it != m_nodes.end());

		it->second->setName(item->text(0).toStdString()	);
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
		for(unsigned pi = 0; pi != node.get().portCount(); ++pi) {
			auto& port = node.get().port(pi);

			m_properties.push_back(Property(port));
			if(m_properties.back().ui) {
				QTreeWidgetItem* portItem = new QTreeWidgetItem();
				portItem->setText(0, port.name().c_str());

				nodeItem->addChild(portItem);

				setItemWidget(portItem, 1, m_properties.back().ui->widget());
			}
		}
	}

	blockSignals(block);

	expandAll();
	resizeColumnToContents(0);
}

////////

Properties::Property::Property(dependency_graph::Port& port) {
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
				if(port.node().graph().network().connections().connectedFrom(port))
					flags |= possumwood::properties::property_base::kDisabled;
			}

			if(port.category() == dependency_graph::Attr::kOutput)
				flags |= possumwood::properties::property_base::kOutput;

			if(port.isDirty())
				flags |= possumwood::properties::property_base::kDirty;

			// and set the flags on the property (calls UI's update, if implemented)
			prop->setFlags(flags);
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

Properties::Property::Property(Property&& prop) {
	ui = std::move(prop.ui);

	graphValueConnection = prop.graphValueConnection;
	prop.graphValueConnection = boost::signals2::connection();

	uiValueConnection = prop.uiValueConnection;
	prop.uiValueConnection = boost::signals2::connection();

	flagsConnection = prop.flagsConnection;
	prop.flagsConnection = boost::signals2::connection();
}

Properties::Property::~Property() {
	graphValueConnection.disconnect();
	uiValueConnection.disconnect();
	flagsConnection.disconnect();

}

