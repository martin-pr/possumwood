#pragma once

#include <QGraphicsTextItem>
#include <QGraphicsTextItem>

class Node;

class Port : public QGraphicsRectItem {
	public:
		enum Type { kInput = 1, kOutput = 2, kInputOutput = 3 };

		Port(const QString& name, Type t, QGraphicsItem* parent);

		const QString name() const;
		const Type portType() const;

	private:
		void setWidth(unsigned w);
		unsigned minWidth() const;

		QGraphicsTextItem* m_name;

		QGraphicsRectItem* m_in;
		QGraphicsRectItem* m_out;

	friend class Node;
};
