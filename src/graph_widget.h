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

		const Node& node(unsigned index) const;
		unsigned nodeCount() const;

		const Node& addNode(const QString& name, const QPointF& position = QPointF(0, 0),
		                    const std::initializer_list<std::pair<QString, Port::Type>>& ports = std::initializer_list<std::pair<QString, Port::Type>>());
		void removeNode(const Node& n);

	private:
		QGraphicsScene* m_scene;
		QGraphicsView* m_view;

		QVector<Node*> m_nodes;
};
