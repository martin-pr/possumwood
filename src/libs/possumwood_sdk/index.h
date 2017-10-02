#pragma once

#include <boost/noncopyable.hpp>

#include <possumwood_sdk/unique_id.h>
#include <possumwood_sdk/drawable.h>
#include <possumwood_sdk/node_data.h>
#include <dependency_graph/node.h>
#include <qt_node_editor/node.h>

namespace possumwood {

/// an indexing structure, allowing to seamlessly switch between the UI classes,
/// and connected data model classes. Data model uses UniqueId instances for
/// indexing, while UI classes just use simple raw pointers to Qt elements.
class Index {
	public:
		struct Item : public boost::noncopyable {
			Item(dependency_graph::Node* gr, node_editor::Node* en, std::unique_ptr<possumwood::Drawable>&& dr) : graphNode(gr), editorNode(en), drawable(std::move(dr)) {
			}

			Item(Item&& i) : graphNode(i.graphNode), editorNode(i.editorNode), drawable(std::move(i.drawable)) {
			}

			dependency_graph::Node* graphNode;
			node_editor::Node* editorNode;
			std::unique_ptr<possumwood::Drawable> drawable;
		};

		void add(Item&& item);

		void remove(const possumwood::UniqueId& id);

		Item& operator[](const possumwood::UniqueId& id);
		const Item& operator[](const possumwood::UniqueId& id) const;

		Item& operator[](node_editor::Node* id);
		const Item& operator[](const node_editor::Node* id) const;

		Item& operator[](dependency_graph::Node* id);
		const Item& operator[](const dependency_graph::Node* id) const;

		typedef std::map<possumwood::UniqueId, Item>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

	private:
		/// this should be implemented using multiindex, but the move semantics in multiindex
		/// library seem a bit iffy in current version of boost.
		std::map<possumwood::UniqueId, Item> m_data;

		std::map<const node_editor::Node*, possumwood::UniqueId> m_uiIndex;
		std::map<const dependency_graph::Node*, possumwood::UniqueId> m_nodeIndex;
};

}
