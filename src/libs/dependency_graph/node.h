#pragma once

#include <functional>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "node_base.h"
#include "attr.h"
#include "metadata.h"
#include "port.h"

namespace dependency_graph {

class Datablock;
class Graph;
class Nodes;
class Network;

class Node : public NodeBase {
	public:
		/// returns the current state of the node (as returned by last compute() evaluation)
		virtual const State& state() const override;

	protected:
		Node(const std::string& name, const MetadataHandle& def, Network* parent);

		virtual void computeInput(size_t index) override;
		virtual void computeOutput(size_t index) override;

	private:
		State m_state;

		friend class Graph;
		friend class Network;
		friend class Port;
		friend class Nodes;

		friend struct io::adl_serializer<Node>;
};

}
