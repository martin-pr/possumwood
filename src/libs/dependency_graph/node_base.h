#pragma once

#include <string>
#include <memory>

#include <boost/noncopyable.hpp>

#include "data.h"
#include "state.h"

namespace dependency_graph {

class Graph;
class Metadata;
class Datablock;
class Node;
class Nodes;
class Network;
class Port;

class NodeBase : public boost::noncopyable {
	public:
		virtual ~NodeBase();

		const std::string& name() const;
		void setName(const std::string& name);

		const Network& network() const;
		Network& network();

		virtual Port& port(size_t index) = 0;
		virtual const Port& port(size_t index) const = 0;
		virtual const size_t portCount() const = 0;

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

		virtual const State& state() const = 0;

		virtual void computeInput(size_t index) = 0;
		virtual void computeOutput(size_t index) = 0;

	protected:
		NodeBase(const std::string& name, Network* parent);

		virtual Datablock& datablock() = 0;
		virtual void setDatablock(const Datablock& data) = 0;

		template<typename T>
		const T& get(size_t index) const;

		template<typename T>
		void set(size_t index, const T& value);

	private:
		void markAsDirty(size_t index);

		std::string m_name;
		Network* m_network;

		std::unique_ptr<BaseData> m_blindData;

		// blind data access
		friend struct io::adl_serializer<Node>;
		friend class Nodes;
		friend class Node;
		friend class Port;
};

}
