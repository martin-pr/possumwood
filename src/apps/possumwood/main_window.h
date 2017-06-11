#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTreeWidget>

#include <boost/filesystem/path.hpp>

#include <dependency_graph/graph.h>

#include <possumwood_sdk/app.h>

#include "adaptor.h"
#include "properties.h"
#include "viewport.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

	private slots:
		void draw(float dt);

	private:
		dependency_graph::Graph m_graph;

		Viewport* m_viewport;
		Adaptor* m_adaptor;
		Properties* m_properties;

		boost::filesystem::path m_filename;
};
