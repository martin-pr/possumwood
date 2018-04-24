#pragma once

#include <string>
#include <memory>

#include <boost/noncopyable.hpp>

#include "data.h"
#include "state.h"
#include "unique_id.h"
#include "metadata.h"
#include "datablock.h"

namespace dependency_graph {

class Graph;
class Node;
class Nodes;
class Network;
class Port;

class NodeBase : public boost::noncopyable {
	public:
		virtual ~NodeBase();

		const std::string& name() const;
		void setName(const std::string& name);

		bool hasParentNetwork() const;
		const Network& network() const;
		Network& network();

		const Graph& graph() const;
		Graph& graph();

		Port& port(size_t index);
		const Port& port(size_t index) const;
		const size_t portCount() const;

		/// returns the unique numeric ID of this node, used for saving connections.
		/// This ID can be used in Graph::operator[] to get this node from the graph.
		UniqueId index() const;

		const Metadata& metadata() const;
		const Datablock& datablock() const;

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

		template<typename T>
		bool is() const;

		template<typename T>
		const T& as() const;

		template<typename T>
		T& as();

	protected:
		NodeBase(const std::string& name, const UniqueId& id, const MetadataHandle& metadata, Network* parent);

		Datablock& datablock();
		void setDatablock(const Datablock& data);

	private:
		// used by Port instances
		template<typename T>
		const T& get(size_t index) const;

		// used by Port instances
		template<typename T>
		void set(size_t index, const T& value);

		// used by Port instances
		void markAsDirty(size_t index);

		std::string m_name;
		Network* m_network;
		UniqueId m_index;

		std::unique_ptr<BaseData> m_blindData;

		MetadataHandle m_metadata;
		Datablock m_data;
		std::vector<Port> m_ports;

		// blind data access
		friend struct io::adl_serializer<Node>;
		friend class Nodes;
		friend class Port;
};

}
