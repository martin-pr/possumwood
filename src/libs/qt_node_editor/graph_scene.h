#pragma once

#include <QGraphicsScene>
#include <memory>
#include <set>

#include "edge.h"
#include "node.h"

namespace node_editor {

class ConnectedEdge;
class Node;

class GraphScene : public QGraphicsScene {
	Q_OBJECT

  public:
	GraphScene(QGraphicsView* parent);
	virtual ~GraphScene();

	bool isReadOnly() const;
	void setReadOnly(bool ro);

	Node& node(unsigned index);
	const Node& node(unsigned index) const;
	unsigned nodeCount() const;

	ConnectedEdge& edge(unsigned index);
	const ConnectedEdge& edge(unsigned index) const;
	unsigned edgeCount() const;

	Node& addNode(const QString& name, const QPointF& position, const QColor& color = QColor(64, 64, 64));

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

  signals:
	void selectionChanged(const Selection&);
	void portsConnected(Port&, Port&);
	void nodesMoved(const std::set<Node*>&);

	void doubleClicked(Node*);
	void middleClicked(Node*);
	void rightClicked(Node*);

  protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

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

	void registerNodeMove(Node*);

	bool m_leftMouseDown;
	std::set<Node*> m_movingNodes;

	bool m_readOnly;

	friend class Edge;
	friend class Node;
};

}  // namespace node_editor
