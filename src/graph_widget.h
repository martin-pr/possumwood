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
		const Node& node(unsigned index) const;
		unsigned nodeCount() const;

		ConnectedEdge& edge(unsigned index);
		const ConnectedEdge& edge(unsigned index) const;
		unsigned edgeCount() const;

		Node& addNode(const QString& name,
		              const std::initializer_list<Node::PortDefinition>& ports = std::initializer_list<Node::PortDefinition>());
		Node& addNode(const QString& name,
		              const std::initializer_list<Node::PortDefinition>& ports,
		              const QPointF& position);
		void removeNode(Node& n);

		void connect(Port& p1, Port& p2);
		void disconnect(Port& p1, Port& p2);
		void disconnect(ConnectedEdge& e);

		// QGraphView is not a QWidget - cannot use Qt's signals/slots.
		// Whatever - let's do it via functors.
		void setContextMenuCallback(std::function<void(QPoint)> fn);
		void setKeyPressCallback(std::function<void(const QKeyEvent&)> fn);

	protected:
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;

	private:
		QGraphicsScene* m_scene;

		QVector<Node*> m_nodes;
		QVector<ConnectedEdge*> m_edges;

		std::function<void(QPoint)> m_contextMenuCallback;
		std::function<void(const QKeyEvent&)> m_keyPressCallback;
};
