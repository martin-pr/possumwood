#pragma once

#include <QMainWindow>
#include <QTreeWidget>

#include <graph.h>

#include "adaptor.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow();

	private:
		dependency_graph::Graph m_graph;

		Adaptor* m_adaptor;
		QTreeWidget* m_properties;

		unsigned m_nodeCounter;
};
