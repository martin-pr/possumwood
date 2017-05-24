#include "properties.h"

#include <QHeaderView>

#include <node.inl>
#include <port.inl>

Properties::Properties(QWidget* parent) : QTreeWidget(parent) {
	// setRootIsDecorated(false);

	headerItem()->setText(0, "item");
	headerItem()->setText(1, "value");
	headerItem()->setFirstColumnSpanned(false);
	header()->setStretchLastSection(true);
}

void Properties::show(const std::vector<std::reference_wrapper<dependency_graph::Node>>& selection) {
	clear();
	m_properties.clear();

	for(const auto& node : selection) {
		// create a top level item for each node
		QTreeWidgetItem* nodeItem = new QTreeWidgetItem();
		nodeItem->setText(0, node.get().name().c_str());

		addTopLevelItem(nodeItem);

		// add each port as a subitem
		for(unsigned pi = 0; pi != node.get().portCount(); ++pi) {
			auto& port = node.get().port(pi);

			QTreeWidgetItem* portItem = new QTreeWidgetItem();
			portItem->setText(0, port.name().c_str());

			nodeItem->addChild(portItem);

			// portItem->setText(1, std::to_string(port.get<float>()).c_str());
			m_properties.push_back(Property(port));
			setItemWidget(portItem, 1, m_properties.back().ui->widget());
		}
	}

	expandAll();
}

////////

Properties::Property::Property(dependency_graph::Port& port) {
	// create the widget
	ui = properties::factories::singleton().create(port.type());
	if(ui) {
		properties::property_base* prop = ui.get();

		// chane of port value
		graphValueConnection = port.valueCallback([prop, &port]() {
			prop->valueFromPort(port);
		});

		// change of port flags
		std::function<void()> updateFlags = [prop, &port]() {
			// build the flags based on port's properties
			unsigned flags = 0;

			if(port.category() == dependency_graph::Attr::kInput) {
				flags |= properties::property_base::kInput;
				if(port.node().graph().connections().connectedFrom(port))
					flags |= properties::property_base::kDisabled;
			}

			if(port.category() == dependency_graph::Attr::kOutput)
				flags |= properties::property_base::kOutput;

			if(port.isDirty())
				flags |= properties::property_base::kDirty;

			// and set the flags on the property (calls UI's update, if implemented)
			prop->setFlags(flags);
		};

		flagsConnection = port.flagsCallback(updateFlags);

		uiValueConnection = ui->valueCallback([prop, &port]() {
			// value changes only for non-disabled input port UIs
			// (avoid Qt's value changed callbacks when not necessary)
			if((prop->flags() & properties::property_base::kInput) && !(prop->flags() & properties::property_base::kDisabled))
				prop->valueToPort(port);
		});

		// run the update functions for the first time to set up the UI
		ui->valueFromPort(port);
		updateFlags();
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

