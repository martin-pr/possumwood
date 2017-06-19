#pragma once

#include <QtWidgets/QTreeWidget>

#include <map>

#include <boost/noncopyable.hpp>

#include <qt_node_editor/node.h>
#include <possumwood_sdk/properties/property.h>
#include <dependency_graph/selection.h>

/// A widget displaying properties of selected nodes
class Properties : public QTreeWidget {
	Q_OBJECT

	public:
		Properties(QWidget* parent = NULL);

		void show(const dependency_graph::Selection& selection);

	protected:
	private:
		struct Property : public boost::noncopyable {
			Property(dependency_graph::Port& port);
			~Property();

			Property(Property&&);

			std::unique_ptr<possumwood::properties::property_base> ui;
			boost::signals2::connection graphValueConnection, uiValueConnection, flagsConnection;
		};

		std::vector<Property> m_properties;
		std::map<QTreeWidgetItem*, dependency_graph::Node*> m_nodes;
};
