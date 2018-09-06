#pragma once

#include <memory>
#include <set>

#include <QGraphicsScene>

#include "node.h"
#include "edge.h"

namespace node_editor {

class ConnectedEdge;
class Node;

class GraphScene : public QGraphicsScene {
	Q_OBJECT

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

		struct Selection {
			std::set<Node*> nodes;
			std::set<ConnectedEdge*> connections;
		};

		void setNodeInfoCallback(std::function<std::string(const Node&)> fn);

	signals:
		void selectionChanged(const Selection&);
		void doubleClicked(Node*);
		void portsConnected(Port&, Port&);
		void nodesMoved(const std::set<Node*>&);

	protected:
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
		virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;

	private slots:
		void onSelectionChanged();

	private:
		void remove(Node* n);
		void remove(Edge* e);

		Port* findConnectionPort(QPointF pos) const;
		QPointF findConnectionPoint(QPointF pos, Port::Type portType) const;

		QVector<Node*> m_nodes;
		QVector<ConnectedEdge*> m_edges;

		Edge* m_editedEdge;
		Port::Type m_connectedSide;

		QGraphicsTextItem* m_infoText;
		QGraphicsRectItem* m_infoRect;

		std::function<std::string(const Node&)> m_nodeInfoCallback;

		void registerNodeMove(Node*);

		bool m_leftMouseDown;
		std::set<Node*> m_movingNodes;

		friend class Edge;
		friend class Node;
};

}
