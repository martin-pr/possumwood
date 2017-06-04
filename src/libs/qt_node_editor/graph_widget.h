#pragma once

#include <QGraphicsView>

#include "node.h"
#include "graph_scene.h"

namespace node_editor {

/// a simple graph widget
class GraphWidget : public QGraphicsView {
		Q_OBJECT

	public:
		GraphWidget(QWidget* parent = NULL);

		GraphScene& scene();

	protected:
		virtual void mouseMoveEvent(QMouseEvent* event) override;

	private:
		GraphScene m_scene;

		int m_mouseX, m_mouseY;
};

}
