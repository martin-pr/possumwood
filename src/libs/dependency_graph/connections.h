#pragma once

#include <boost/noncopyable.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/optional.hpp>
#include <boost/iterator/transform_iterator.hpp>

namespace dependency_graph {

class Graph;
class Port;
class Node;

/// A simple container class for all connections. It stores connections as pointers to related
/// ports and does *not* ensure that these are valid in any way. An external mechanism needs
/// to call appropriate functions when needed.
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

}
