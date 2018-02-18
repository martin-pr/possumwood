#pragma once

#include <vector>
#include <functional>

#include <boost/noncopyable.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/optional.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/signals2.hpp>

#include "node.h"
#include "nodes.h"

namespace dependency_graph {

/// The graph data structure - holds node instances and connections.
class Graph : public boost::noncopyable {
	public:
		Graph();

		bool empty() const;
		void clear();

		Nodes& nodes();
		const Nodes& nodes() const;

		class Connections : public boost::noncopyable {
			public:
				/// returns the connections of an input port (result contains references
				///   to output ports connected to this input)
				boost::optional<const Port&> connectedFrom(const Port& p) const;
				/// returns the connections of an output port (result contains references
				///   to input ports connected to this output)
				std::vector<std::reference_wrapper<const Port>> connectedTo(const Port& p) const;

				/// returns the connections of an input port (result contains references
				///   to output ports connected to this input)
				boost::optional<Port&> connectedFrom(Port& p);
				/// returns the connections of an output port (result contains references
				///   to input ports connected to this output)
				std::vector<std::reference_wrapper<Port>> connectedTo(Port& p);

				/// returns the total number of valid connections in this graph
				size_t size() const;

				/// true if this graph contains no connections
				bool empty() const;

				/// the container for connection data (a helper define - not useable outside the implementation cpp)
				typedef
					boost::bimap<
						boost::bimaps::multiset_of<Port*>, // output
						Port* // input (only one output to any input)
					> connections_container;

				/// allows iteration over connections
				typedef boost::transform_iterator<
					std::function<const std::pair<const Port&, const Port&>(const connections_container::left_value_type&)>,
					connections_container::left_const_iterator
				> const_iterator;

				/// connections iteration
				const_iterator begin() const;
				/// connections iteration
				const_iterator end() const;

				/// allows iteration over connections
				typedef boost::transform_iterator<
					std::function<const std::pair<Port&, Port&>(const connections_container::left_value_type&)>,
					connections_container::left_const_iterator
				> iterator;

				/// connections iteration
				iterator begin();
				/// connections iteration
				iterator end();

			private:
				Connections(Graph* parent);

				void add(Port& src, Port& dest);
				void remove(Port& src, Port& dest);
				/// remove all connections related to a node (both in and out)
				void purge(const Node& n);

				Graph* m_parent;
				connections_container m_connections;

				friend class Graph;
				friend class Port;
				friend class Nodes;
		};

		Connections& connections();
		const Connections& connections() const;

		boost::signals2::connection onAddNode(std::function<void(Node&)> callback);
		boost::signals2::connection onRemoveNode(std::function<void(Node&)> callback);

		boost::signals2::connection onConnect(std::function<void(Port&, Port&)> callback);
		boost::signals2::connection onDisconnect(std::function<void(Port&, Port&)> callback);

		boost::signals2::connection onBlindDataChanged(std::function<void(Node&)> callback);
		boost::signals2::connection onNameChanged(std::function<void(Node&)> callback);

		/// dirtiness callback - called when any dirty flag of any port changes (usable for viewport refresh)
		boost::signals2::connection onDirty(std::function<void()> callback);
		/// per-node state change callback
		boost::signals2::connection onStateChanged(std::function<void(const Node&)> callback);
		/// log callback
		boost::signals2::connection onLog(std::function<void(State::MessageType, const std::string&)> callback);

	private:
		std::unique_ptr<Node> makeNode(const std::string& name, const Metadata* md);

		Nodes m_nodes;
		Connections m_connections;

		boost::signals2::signal<void(Node&)> m_onAddNode, m_onRemoveNode, m_onBlindDataChanged, m_onNameChanged;
		boost::signals2::signal<void(Port&, Port&)> m_onConnect, m_onDisconnect;
		boost::signals2::signal<void()> m_onDirty;
		boost::signals2::signal<void(const Node&)> m_onStateChanged;
		boost::signals2::signal<void(State::MessageType, const std::string&)> m_onLog;

		friend class Node;
		friend class Nodes;
		friend class Connections;
};

}
