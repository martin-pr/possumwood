#pragma once

#include <string>
#include <memory>

#include <boost/optional.hpp>

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

class NodeBase {
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
		size_t portCount() const;

		/// returns the unique numeric ID of this node, used for saving connections.
		/// This ID can be used in Graph::operator[] to get this node from the graph.
		UniqueId index() const;

		/// returns Metadata instance
		const MetadataHandle& metadata() const;

		/// sets a new metadata instance. Throws if any ports are currently connected.
		/// Uses an AttrMap instance to map between original and new ports.
		void setMetadata(const MetadataHandle& handle);

		/// returns datablock instance - contains all input and output data
		const Datablock& datablock() const;
		/// sets the datablock instance - new new value has to share the same metadata instance
		void setDatablock(const Datablock& data);

		/// blind per-node data, to be used by the client application
		///   to store visual information (e.g., node position, colour...)
		template<typename T>
		void setBlindData(const T& value);
		/// blind per-node data, to be used by the client application
		///   to store visual information (e.g., node position, colour...)
		template<typename T>
		const T& blindData() const;

		const Data& blindData() const;

		void setBlindData(const Data& data);
		bool hasBlindData() const;
		std::string blindDataType() const;

		const State& state() const;

		void computeInput(size_t index);
		void computeOutput(size_t index);

		template<typename T>
		bool is() const {
			return dynamic_cast<const T*>(this) != nullptr;
		}

		template<typename T>
		const T& as() const;

		template<typename T>
		T& as();

	protected:
		NodeBase(const std::string& name, const UniqueId& id, const MetadataHandle& metadata, Network* parent);

		Datablock& datablock();

	private:
		NodeBase(const NodeBase&) = delete;
		NodeBase& operator = (const NodeBase&) = delete;

		// used by Port instances
		const Data& get(size_t index) const;

		// used by Port instances
		void set(size_t index, const Data& value);

		// used by Port instances
		void markAsDirty(size_t portIndex, bool dependantsOnly = false);

		// used during destruction
		void disconnectAll();

		std::string m_name;
		Network* m_network;
		UniqueId m_index;

		Data m_blindData;

		MetadataHandle m_metadata;
		Datablock m_data;
		std::vector<Port> m_ports;

		State m_state;

		// blind data access
		// friend struct io::adl_serializer<NodeBase>;
		friend class Nodes;
		friend class Port;
};

}
