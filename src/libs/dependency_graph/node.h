#pragma once

#include <functional>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "attr.h"
#include "metadata.h"
#include "port.h"

namespace dependency_graph {

class Datablock;
class Graph;

class Node : public boost::noncopyable {
	public:
		const std::string& name() const;
		void setName(const std::string& name);

		const Metadata& metadata() const;

		/// blind per-node data, to be used by the client application
		///   to store visual information (e.g., node position, colour...)
		template<typename T>
		void setBlindData(const T& value);

		/// blind per-node data, to be used by the client application
		///   to store visual information (e.g., node position, colour...)
		template<typename T>
		const T& blindData() const;

		Port& port(size_t index);
		const Port& port(size_t index) const;
		const size_t portCount() const;

		const Graph& graph() const;
		Graph& graph();

		/// returns the unique numeric ID of this node, used for saving connections.
		/// This ID can be used in Graph::operator[] to get this node from the graph.
		size_t index() const;

	protected:
		Node(const std::string& name, const Metadata* def, Graph* parent);

		void computeInput(size_t index);
		void computeOutput(size_t index);

		template<typename T>
		const T& get(size_t index) const;

		template<typename T>
		void set(size_t index, const T& value);

		bool inputIsConnected(const Port& p) const;

	private:
		void markAsDirty(size_t index);

		std::string m_name;
		Graph* m_parent;

		const Metadata* m_meta;
		Datablock m_data;
		std::unique_ptr<BaseData> m_blindData;

		std::vector<Port> m_ports;

		friend class Graph;
		friend class Port;

		friend class io::adl_serializer<Node>;
};

}
