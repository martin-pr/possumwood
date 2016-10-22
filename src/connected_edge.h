#pragma once

#include "edge.h"

class Port;

class ConnectedEdge : public Edge {
	public:
		ConnectedEdge(Port& p1, Port& p2);

		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

		const Port& fromPort() const;
		const Port& toPort() const;

	private:
		void adjust();

		Port* m_p1, *m_p2;

		friend class Port;
};
