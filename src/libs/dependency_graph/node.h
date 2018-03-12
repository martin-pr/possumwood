#pragma once

#include <functional>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "node_base.h"
#include "attr.h"
#include "metadata.h"
#include "port.h"
#include "state.h"

namespace dependency_graph {

class Datablock;
class Graph;
class Nodes;

class Node : public NodeBase {
	public:
		virtual const Metadata& metadata() const override;
		/// only useful for copy-paste - needs to replicate a node with its data
		virtual const Datablock& datablock() const override;

		Port& port(size_t index);
		const Port& port(size_t index) const;
		const size_t portCount() const;

		/// returns the current state of the node (as returned by last compute() evaluation)
		const State& state() const;

	protected:
		Node(const std::string& name, const Metadata* def, Graph* parent);

		void computeInput(size_t index);
		void computeOutput(size_t index);

		template<typename T>
		const T& get(size_t index) const;

		template<typename T>
		void set(size_t index, const T& value);

	private:
		void markAsDirty(size_t index);

		const Metadata* m_meta;
		Datablock m_data;

		std::vector<Port> m_ports;

		State m_state;

		friend class Graph;
		friend class Port;
		friend class Nodes;

		friend struct io::adl_serializer<Node>;
};

}
