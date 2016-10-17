#pragma once

#include <QGraphicsLineItem>

class Port;

class Edge : public QGraphicsLineItem {
	public:
		Edge(Port& p1, Port& p2);

	private:
		void adjust();

		Port *m_p1, *m_p2;

	friend class Port;
};
