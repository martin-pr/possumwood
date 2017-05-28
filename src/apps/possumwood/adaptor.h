#pragma once

#include <vector>

#include <boost/signals2.hpp>
#include <boost/bimap.hpp>

#include <QWidget>

#include <dependency_graph/graph.h>
#include "graph_widget.h"

/// A simple adaptor widget, marrying qt_graph_editor and dependency_graph
class Adaptor : public QWidget {
		Q_OBJECT

	public:
		/// initialised with a graph instance (NOT taking ownership of it!)
		Adaptor(dependency_graph::Graph* graph);
		virtual ~Adaptor();

		/// maps a position in widget space to scene space
		QPointF mapToScene(QPoint pos) const;

		/// returns the scene instance
		node_editor::GraphScene& scene();
		const node_editor::GraphScene& scene() const;

		/// returns the dependency graph
		dependency_graph::Graph& graph();

		/// deletes all selected items in the associated graph
		void deleteSelected();

		/// returns a list of all selected nodes
		std::vector<std::reference_wrapper<dependency_graph::Node>> selectedNodes();

	protected:
	private:
		void onAddNode(dependency_graph::Node& node);
		void onRemoveNode(dependency_graph::Node& node);

		void onConnect(dependency_graph::Port& p1, dependency_graph::Port& p2);
		void onDisconnect(dependency_graph::Port& p1, dependency_graph::Port& p2);

		void onBlindDataChanged(dependency_graph::Node& node);

		dependency_graph::Graph* m_graph;
		node_editor::GraphWidget* m_graphWidget;

		std::vector<boost::signals2::connection> m_signals;

		boost::bimap<dependency_graph::Node*, node_editor::Node*> m_nodes;
};
