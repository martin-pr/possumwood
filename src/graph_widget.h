#pragma once

#include <functional>

#include <QGraphicsScene>
#include <QGraphicsView>

#include "node.h"

/// a simple graph widget
class GraphWidget : public QGraphicsView {
		Q_OBJECT

	public:
		GraphWidget(QWidget* parent);

		void clear();

		Node& node(unsigned index);
		unsigned nodeCount() const;

		Node& addNode(const QString& name,
		              const std::initializer_list<Node::PortDefinition>& ports = std::initializer_list<Node::PortDefinition>());
		Node& addNode(const QString& name,
		              const std::initializer_list<Node::PortDefinition>& ports,
		              const QPointF& position);
		void removeNode(Node& n);

		void connect(Port& p1, Port& p2);

		// QGraphView is not a QWidget - cannot use Qt's signals/slots.
		// Whatever - let's do it via functors.
		void setContextMenuCallback(std::function<void(QPoint)> fn);

	protected:
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;

	private:
		QGraphicsScene* m_scene;

		QVector<Node*> m_nodes;
		QVector<Edge*> m_edges;

		std::function<void(QPoint)> m_contextMenuCallback;
};
