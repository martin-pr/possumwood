#pragma once

#include <vector>

#include <boost/signals2.hpp>
#include <boost/bimap.hpp>

#include <QWidget>
#include <QAction>

#include <possumwood_sdk/drawable.h>
#include <dependency_graph/graph.h>
#include <dependency_graph/selection.h>
#include <qt_node_editor/graph_widget.h>

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

		node_editor::GraphWidget* graphWidget();

		/// returns current selection
		dependency_graph::Selection selection() const;
		void setSelection(const dependency_graph::Selection& selection);

		void setSizeHint(const QSize& sh);
		virtual QSize sizeHint() const override;

		QAction* cutAction() const;
		QAction* copyAction() const;
		QAction* pasteAction() const;
		QAction* deleteAction() const;

		QAction* undoAction() const;
		QAction* redoAction() const;

		/// calls all existing Drawables
		void draw(const possumwood::Drawable::ViewportState& viewport);

	protected:
	private:
		void onAddNode(dependency_graph::Node& node);
		void onRemoveNode(dependency_graph::Node& node);

		void onConnect(dependency_graph::Port& p1, dependency_graph::Port& p2);
		void onDisconnect(dependency_graph::Port& p1, dependency_graph::Port& p2);

		void onBlindDataChanged(dependency_graph::NodeBase& node);
		void onNameChanged(dependency_graph::NodeBase& node);
		void onStateChanged(const dependency_graph::Node& node);

		dependency_graph::Graph* m_graph;
		node_editor::GraphWidget* m_graphWidget;

		std::vector<boost::signals2::connection> m_signals;

		QSize m_sizeHint;

		QAction *m_copy, *m_paste, *m_cut, *m_delete, *m_undo, *m_redo;
};
