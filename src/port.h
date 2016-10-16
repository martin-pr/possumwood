#pragma once

#include <QGraphicsTextItem>
#include <QGraphicsTextItem>

class Port : public QGraphicsRectItem {
	public:
		enum Type { kInput = 1, kOutput = 2, kInputOutput = 3 };

		Port(const QString& name, Type t, QGraphicsItem* parent);

		void setWidth(unsigned w);
		unsigned minWidth() const;

	private:
		QGraphicsTextItem* m_name;

		QGraphicsRectItem* m_in;
		QGraphicsRectItem* m_out;
};
