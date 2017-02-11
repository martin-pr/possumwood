#pragma once

#include <QMainWindow>

#include <graph.h>

class MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow();

	private:
		dependency_graph::Graph m_graph;

		unsigned m_nodeCounter;
};
