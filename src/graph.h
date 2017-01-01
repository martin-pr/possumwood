#pragma once

#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/optional.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include "node.h"

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

				Node& add(const Metadata& type, const std::string& name);
				iterator erase(iterator i);

			private:
				Nodes(Graph* parent);

				Graph* m_parent;

				// stored in a pointer container, to keep parent pointers
				//   stable without too much effort (might change)
				std::vector<std::unique_ptr<Node>> m_nodes;

				friend class Graph;
		};

		Nodes& nodes();
		const Nodes& nodes() const;

		class Connections : public boost::noncopyable {
			public:
				/// returns the connections of an input port (result contains references
				///   to output ports connected to this input)
				boost::optional<const Node::Port&> connectedFrom(const Node::Port& p) const;
				/// returns the connections of an output port (result contains references
				///   to input ports connected to this output)
				std::vector<std::reference_wrapper<const Node::Port>> connectedTo(const Node::Port& p) const;

				/// returns the connections of an input port (result contains references
				///   to output ports connected to this input)
				boost::optional<Node::Port&> connectedFrom(Node::Port& p);
				/// returns the connections of an output port (result contains references
				///   to input ports connected to this output)
				std::vector<std::reference_wrapper<Node::Port>> connectedTo(Node::Port& p);

			private:
				Connections() = default;

				void add(Node::Port& src, Node::Port& dest);

				boost::bimap<
					boost::bimaps::multiset_of<Node::Port*>, // output
					Node::Port* // input (only one output to any input)
				> m_connections;

				friend class Graph;
				friend class Node::Port;
		};

		Connections& connections();
		const Connections& connections() const;

	private:
		std::unique_ptr<Node> makeNode(const std::string& name, const Metadata* md);

		Nodes m_nodes;
		Connections m_connections;

};
