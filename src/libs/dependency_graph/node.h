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
		virtual const MetadataHandle& metadata() const override;
		/// only useful for copy-paste - needs to replicate a node with its data
		virtual const Datablock& datablock() const override;

		virtual Port& port(size_t index) override;
		virtual const Port& port(size_t index) const override;
		virtual const size_t portCount() const override;

		/// returns the current state of the node (as returned by last compute() evaluation)
		virtual const State& state() const override;

	protected:
		Node(const std::string& name, const MetadataHandle& def, Network* parent);

		virtual void setDatablock(const Datablock& data) override;
		virtual Datablock& datablock() override;

		virtual void computeInput(size_t index) override;
		virtual void computeOutput(size_t index) override;

	private:
		MetadataHandle m_meta;
		Datablock m_data;

		std::vector<Port> m_ports;

		State m_state;

		friend class Graph;
		friend class Network;
		friend class Port;
		friend class Nodes;

		friend struct io::adl_serializer<Node>;
};

}
