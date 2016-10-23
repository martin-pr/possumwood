#pragma once

#include <QGraphicsScene>

#include "node.h"

class ConnectedEdge;

class GraphScene : public QGraphicsScene {
	public:
		GraphScene(QGraphicsView* parent);

		void clear();

		Node& node(unsigned index);
		const Node& node(unsigned index) const;
		unsigned nodeCount() const;

		ConnectedEdge& edge(unsigned index);
		const ConnectedEdge& edge(unsigned index) const;
		unsigned edgeCount() const;

		// Node& addNode(const QString& name,
		//               const std::initializer_list<Node::PortDefinition>& ports = std::initializer_list<Node::PortDefinition>());
		Node& addNode(const QString& name,
		              const std::initializer_list<Node::PortDefinition>& ports,
		              const QPointF& position);
		void removeNode(Node& n);

		void connect(Port& p1, Port& p2);
		void disconnect(Port& p1, Port& p2);
		void disconnect(ConnectedEdge& e);

	protected:
	private:
		QVector<Node*> m_nodes;
		QVector<ConnectedEdge*> m_edges;
};
