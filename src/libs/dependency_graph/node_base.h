#pragma once

#include <string>
#include <memory>

#include <boost/noncopyable.hpp>

#include "data.h"

namespace dependency_graph {

class Graph;
class Metadata;
class Datablock;
class Node;
class Nodes;

class NodeBase : public boost::noncopyable {
	public:
		virtual ~NodeBase();

		const std::string& name() const;
		void setName(const std::string& name);

		const Graph& graph() const;
		Graph& graph();

		/// returns the unique numeric ID of this node, used for saving connections.
		/// This ID can be used in Graph::operator[] to get this node from the graph.
		size_t index() const;

		virtual const Metadata& metadata() const = 0;
		virtual const Datablock& datablock() const = 0;

		/// blind per-node data, to be used by the client application
		///   to store visual information (e.g., node position, colour...)
		template<typename T>
		void setBlindData(const T& value);

		/// blind per-node data, to be used by the client application
		///   to store visual information (e.g., node position, colour...)
		template<typename T>
		const T& blindData() const;

	protected:
		NodeBase(const std::string& name, Graph* parent);

	private:
		std::string m_name;
		Graph* m_graph;

		std::unique_ptr<BaseData> m_blindData;

		// blind data access
		friend struct io::adl_serializer<Node>;
		friend class Nodes;
};

}
