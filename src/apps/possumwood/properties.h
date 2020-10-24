#pragma once

#include <map>

#include <boost/bimap.hpp>
#include <boost/noncopyable.hpp>

#include <QtWidgets/QTreeWidget>

#include <dependency_graph/selection.h>
#include <qt_node_editor/node.h>

#include <possumwood_sdk/properties/property.h>

/// A widget displaying properties of selected nodes
class Properties : public QTreeWidget {
	Q_OBJECT

  public:
	Properties(QWidget* parent = NULL);
	virtual ~Properties();

	void show(const dependency_graph::Selection& selection);

  protected:
	void onMetadataChanged(dependency_graph::NodeBase& n);

  private:
	struct PropertyHolder : public boost::noncopyable {
		PropertyHolder(dependency_graph::Port& port);
		~PropertyHolder();

		PropertyHolder(PropertyHolder&&);

		std::unique_ptr<possumwood::properties::property_base> ui;
		boost::signals2::connection graphValueConnection, uiValueConnection, flagsConnection;
	};

	std::vector<PropertyHolder> m_properties;
	boost::bimap<QTreeWidgetItem*, dependency_graph::NodeBase*> m_nodes;

	boost::signals2::connection m_metadataConnection;
};
