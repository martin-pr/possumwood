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

namespace dependency_graph {

/// The graph data structure - holds node instances and connections.
class Graph : public boost::noncopyable {
	public:
		Graph();

		bool empty() const;
		void clear();

		/// Data structure holding node instances.
		/// Iterators are not guaranteed to remain valid after operations,
		/// but the node instances are stored in a vector container (their
		/// positions in the array will not change after add, and erase
		/// shifts subsequent indices), and each node instance's memory
		/// address is guaranteed not to change during its lifetime (Nodes
		/// are stored as pointers).
		class Nodes : public boost::noncopyable {
			public:
				bool empty() const;
				std::size_t size() const;

				Node& operator[](std::size_t index);
				const Node& operator[](std::size_t index) const;

				typedef boost::indirect_iterator<std::vector<std::unique_ptr<Node>>::const_iterator> const_iterator;
				const_iterator begin() const;
				const_iterator end() const;

				typedef boost::indirect_iterator<std::vector<std::unique_ptr<Node>>::iterator> iterator;
				iterator begin();
				iterator end();

				Node& add(const Metadata& type, const std::string& name, std::unique_ptr<BaseData>&& blindData = std::unique_ptr<BaseData>());

				template<typename T>
				Node& add(const Metadata& type, const std::string& name, const T& blindData);

				iterator erase(iterator i);
				void clear();

			private:
				Nodes(Graph* parent);

				size_t findNodeIndex(const Node& n) const;

				Graph* m_parent;

				// stored in a pointer container, to keep parent pointers
				//   stable without too much effort (might change)
				std::vector<std::unique_ptr<Node>> m_nodes;

				friend class Graph;
				friend class Node;
		};

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

		boost::signals2::connection onDirty(std::function<void()> callback);

	private:
		std::unique_ptr<Node> makeNode(const std::string& name, const Metadata* md);

		Nodes m_nodes;
		Connections m_connections;

		boost::signals2::signal<void(Node&)> m_onAddNode, m_onRemoveNode, m_onBlindDataChanged, m_onNameChanged;
		boost::signals2::signal<void(Port&, Port&)> m_onConnect, m_onDisconnect;
		boost::signals2::signal<void()> m_onDirty;

		friend class Node;
		friend class Nodes;
		friend class Connections;
};

/////////

template<typename T>
Node& Graph::Nodes::add(const Metadata& type, const std::string& name, const T& blindData) {
	m_nodes.push_back(m_parent->makeNode(name, &type));

	m_nodes.back()->m_blindData = std::unique_ptr<BaseData>(
		new Data<T>{blindData});

	m_parent->m_onAddNode(*m_nodes.back());
	m_parent->m_onDirty();

	return *m_nodes.back();
}

}
