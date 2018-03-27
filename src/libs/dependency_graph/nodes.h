#pragma once

#include <set>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/optional.hpp>

#include "data.h"
#include "node.h"

namespace dependency_graph {

class Graph;
class NodeBase;
class Node;
class Metadata;
class Datablock;
class Network;

/// Data structure holding node instances.
/// Iterators are not guaranteed to remain valid after operations,
/// but the node instances are stored in a vector container (their
/// positions in the array will not change after add, and erase
/// shifts subsequent indices), and each node instance's memory
/// address is guaranteed not to change during its lifetime (Nodes
/// are stored as pointers).
class Nodes : public boost::noncopyable {
	private:
		struct Compare {
			bool operator()(const std::unique_ptr<NodeBase>& n1, const std::unique_ptr<NodeBase>& n2) const {
				return n1->index() < n2->index();
			}
			bool operator()(const UniqueId& n1, const std::unique_ptr<NodeBase>& n2) const {
				return n1 < n2->index();
			}
			bool operator()(const std::unique_ptr<NodeBase>& n1, const UniqueId& n2) const {
				return n1->index() < n2;
			}
			typedef bool is_transparent;
		};

		using NodeSet = std::set<std::unique_ptr<NodeBase>, Compare>;

	public:
		bool empty() const;
		std::size_t size() const;

		NodeBase& operator[](const UniqueId& index);
		const NodeBase& operator[](const UniqueId& index) const;

		typedef boost::indirect_iterator<NodeSet::const_iterator> const_iterator;
		const_iterator begin() const;
		const_iterator end() const;
		const_iterator find(const UniqueId& id) const;

		typedef boost::indirect_iterator<NodeSet::iterator> iterator;
		iterator begin();
		iterator end();
		iterator find(const UniqueId& id);

		NodeBase& add(const MetadataHandle& type, const std::string& name,
		              std::unique_ptr<BaseData>&& blindData = std::unique_ptr<BaseData>(),
		              boost::optional<const dependency_graph::Datablock&> datablock = boost::optional<const dependency_graph::Datablock&>());

		template<typename T>
		NodeBase& add(const MetadataHandle& type, const std::string& name, const T& blindData,
		              boost::optional<const dependency_graph::Datablock&> datablock = boost::optional<const dependency_graph::Datablock&>());

		iterator erase(iterator i);
		void clear();

	private:
		Nodes(Network* parent);

		Network* m_parent;

		// stored in a pointer container, to keep parent pointers
		//   stable without too much effort (might change)
		NodeSet m_nodes;

		friend class Graph;
		friend class NodeBase;
		friend class Node;
		friend class Network;
};

}
