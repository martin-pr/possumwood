#pragma once

#include <vector>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/optional.hpp>

#include "data.h"

namespace dependency_graph {

class Graph;
class Node;
class Metadata;
class Datablock;

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

		Node& add(const Metadata& type, const std::string& name, std::unique_ptr<BaseData>&& blindData = std::unique_ptr<BaseData>(), boost::optional<const dependency_graph::Datablock&> datablock = boost::optional<const dependency_graph::Datablock&>());

		template<typename T>
		Node& add(const Metadata& type, const std::string& name, const T& blindData, boost::optional<const dependency_graph::Datablock&> datablock = boost::optional<const dependency_graph::Datablock&>());

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

}
