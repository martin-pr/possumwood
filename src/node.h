#pragma once

#include <string>

#include <QGraphicsItem>
#include <QGraphicsTextItem>

class Node : public QGraphicsItem {
	public:
		Node(const QString& name, const QPointF& position = QPointF(0,0));

		void setName(const QString& n);
		const QString name() const;

		virtual QRectF boundingRect() const override;
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = NULL) override;

	protected:
	private:
		QGraphicsTextItem* m_title;
		// QVector<QGraphicsItem>* m_ports;
};
