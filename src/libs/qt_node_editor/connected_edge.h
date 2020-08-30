#pragma once

#include "edge.h"

namespace node_editor {

class Port;

class ConnectedEdge : public Edge {
  public:
	ConnectedEdge(Port& p1, Port& p2);
	virtual ~ConnectedEdge();

	const Port& fromPort() const;
	Port& fromPort();
	const Port& toPort() const;
	Port& toPort();

  private:
	void adjust();

	Port *m_p1, *m_p2;

	friend class Port;
};

}  // namespace node_editor
