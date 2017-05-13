#pragma once

#include <QtWidgets/QTreeWidget>

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
		std::vector<std::unique_ptr<properties::property_base>> m_properties;
};
