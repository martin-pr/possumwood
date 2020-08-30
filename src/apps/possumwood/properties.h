#pragma once

#include <dependency_graph/selection.h>
#include <possumwood_sdk/properties/property.h>
#include <qt_node_editor/node.h>

#include <QtWidgets/QTreeWidget>
#include <boost/noncopyable.hpp>
#include <map>

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

	std::vector<PropertyHolder> m_properties;
	std::map<QTreeWidgetItem*, dependency_graph::NodeBase*> m_nodes;
};
