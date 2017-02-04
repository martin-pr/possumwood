#pragma once

#include <memory>

#include <QGraphicsScene>

#include "node.h"
#include "edge.h"

namespace node_editor {

class ConnectedEdge;
class Node;

class GraphScene : public QGraphicsScene {
	public:
		GraphScene(QGraphicsView* parent);
		virtual ~GraphScene();

		Node& node(unsigned index);
		const Node& node(unsigned index) const;
		unsigned nodeCount() const;

		ConnectedEdge& edge(unsigned index);
		const ConnectedEdge& edge(unsigned index) const;
		unsigned edgeCount() const;

		Node& addNode(const QString& name,
		              const QPointF& position,
		              const std::initializer_list<Node::PortDefinition>& ports = std::initializer_list<Node::PortDefinition>());

		void removeNode(Node& n);

		void connect(Port& p1, Port& p2);
		void disconnect(Port& p1, Port& p2);
		void disconnect(ConnectedEdge& e);
		bool isConnected(const Port& p1, const Port& p2);

		bool isEdgeEditInProgress() const;
		/// node connection changes callback (the only graph change necessary
		///   out of the box)
		void setMouseConnectionCallback(std::function<void(Port&, Port&)> fn);

		void setNodeMoveCallback(std::function<void(Node&)> fn);

	protected:
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	private:
		void remove(Node* n);
		void remove(Edge* e);

		Port* findConnectionPort(QPointF pos) const;
		QPointF findConnectionPoint(QPointF pos, Port::Type portType) const;

		QVector<Node*> m_nodes;
		QVector<ConnectedEdge*> m_edges;

		Edge* m_editedEdge;
		Port::Type m_connectedSide;

		std::function<void(Port&, Port&)> m_connectionCallback;
		std::function<void(Node&)> m_nodeMoveCallback;

		friend class Edge;
		friend class Node;
};

}
