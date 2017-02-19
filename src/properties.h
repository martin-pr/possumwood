#pragma once

#include <QTreeWidget>

#include <node.h>

class Properties : public QTreeWidget {
	Q_OBJECT

	public:
		Properties(QWidget* parent = NULL);

		void show(const std::vector<std::reference_wrapper<dependency_graph::Node>>& nodes);

	protected:
	private:
};
