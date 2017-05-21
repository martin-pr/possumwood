#pragma once

#include <QtWidgets/QTreeWidget>

#include <boost/noncopyable.hpp>

#include <node.h>
#include <properties_ui/property.h>

/// A widget displaying properties of selected nodes
class Properties : public QTreeWidget {
	Q_OBJECT

	public:
		Properties(QWidget* parent = NULL);

		void show(const std::vector<std::reference_wrapper<dependency_graph::Node>>& nodes);

	protected:
	private:
		struct Property : public boost::noncopyable {
			Property(dependency_graph::Node::Port& port);
			~Property();

			Property(Property&&);

			std::unique_ptr<properties::property_base> ui;
			boost::signals2::connection graphValueConnection, uiValueConnection, flagsConnection;
		};

		std::vector<Property> m_properties;
};
