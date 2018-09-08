#pragma once

#include <QtWidgets/QTreeWidget>

#include <map>

#include <boost/noncopyable.hpp>

#include <qt_node_editor/node.h>
#include <possumwood_sdk/properties/property.h>
#include <dependency_graph/selection.h>

#include "status.h"

/// A widget displaying properties of selected nodes
class Properties : public QTreeWidget {
	Q_OBJECT

	public:
		Properties(QWidget* parent = NULL);

		void show(const dependency_graph::Selection& selection);

	protected:
	private:
		struct PropertyHolder : public boost::noncopyable {
			PropertyHolder(dependency_graph::Port& port);
			~PropertyHolder();

			PropertyHolder(PropertyHolder&&);

			std::unique_ptr<possumwood::properties::property_base> ui;
			boost::signals2::connection graphValueConnection, uiValueConnection, flagsConnection;
		};

		struct StatusHolder : public boost::noncopyable {
			StatusHolder(dependency_graph::NodeBase& node);
			~StatusHolder();

			StatusHolder(StatusHolder&&);

			std::unique_ptr<Status> m_status;
		};

		std::vector<PropertyHolder> m_properties;
		std::vector<StatusHolder> m_states;
		std::map<QTreeWidgetItem*, dependency_graph::NodeBase*> m_nodes;
};
