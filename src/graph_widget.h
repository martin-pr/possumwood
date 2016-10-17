#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>

#include "node.h"

/// a simple graph widget
class GraphWidget : public QWidget {
	Q_OBJECT

	public:
		GraphWidget(QWidget* parent);

		void clear();

		Node& node(unsigned index);
		unsigned nodeCount() const;

		Node& addNode(const QString& name, const QPointF& position = QPointF(0, 0),
		              const std::initializer_list<std::pair<QString, Port::Type>>& ports = std::initializer_list<std::pair<QString, Port::Type>>());
		void removeNode(Node& n);

		void connect(Port& p1, Port& p2);

	private:
		QGraphicsScene* m_scene;
		QGraphicsView* m_view;

		QVector<Node*> m_nodes;
		QVector<Edge*> m_edges;
};
