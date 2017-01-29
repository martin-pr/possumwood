#pragma once

#include <functional>

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

		// QGraphView is not a QWidget - cannot use Qt's signals/slots.
		// Whatever - let's do it via functors.
		void setContextMenuCallback(std::function<void(QPoint)> fn);
		void setKeyPressCallback(std::function<void(const QKeyEvent&)> fn);

	protected:
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;

	private:
		GraphScene m_scene;

		std::function<void(QPoint)> m_contextMenuCallback;
		std::function<void(const QKeyEvent&)> m_keyPressCallback;
};

}
