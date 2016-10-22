#pragma once

#include <QGraphicsTextItem>
#include <QGraphicsTextItem>
#include <QSet>

class Node;
class Edge;

class Port : public QGraphicsRectItem {
	public:
		enum Type { kInput = 1, kOutput = 2, kInputOutput = 3 };

		Port(const QString& name, Type t, QColor color, Node* parent);

		const QString name() const;
		const Type portType() const;
		const QColor color() const;

		Node& parentNode();
		const Node& parentNode() const;

	private:
		void setWidth(unsigned w);
		unsigned minWidth() const;

		void adjustEdges();

		QGraphicsTextItem* m_name;

		QColor m_color;

		QGraphicsRectItem* m_in;
		QGraphicsRectItem* m_out;

		QSet<Edge*> m_edges;
		Node* m_parent;

		friend class Node;
		friend class Edge;
};
