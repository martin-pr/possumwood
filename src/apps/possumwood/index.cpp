#include "index.h"

void Index::add(Item&& item) {
	const possumwood::UniqueId id = item.graphNode->blindData<possumwood::NodeData>().id();

	assert(m_data.find(id) == m_data.end());
	auto it = m_data.insert(std::make_pair(id, std::move(item))).first;

	assert(m_uiIndex.find(it->second.editorNode) == m_uiIndex.end());
	m_uiIndex.insert(std::make_pair(it->second.editorNode, id));
}

void Index::remove(const possumwood::UniqueId& id) {
	auto it = m_data.find(id);
	assert(it != m_data.end());

	auto uiIt = m_uiIndex.find(it->second.editorNode);
	assert(uiIt != m_uiIndex.end());
	m_uiIndex.erase(uiIt);

	m_data.erase(it);
}

Index::Item& Index::operator[](const possumwood::UniqueId& id) {
	auto it = m_data.find(id);
	assert(it != m_data.end());
	return it->second;
}

const Index::Item& Index::operator[](const possumwood::UniqueId& id) const {
	auto it = m_data.find(id);
	assert(it != m_data.end());
	return it->second;
}

Index::Item& Index::operator[](node_editor::Node* ptr) {
	auto uiIt = m_uiIndex.find(ptr);
	assert(uiIt != m_uiIndex.end());

	auto it = m_data.find(uiIt->second);
	assert(it != m_data.end());
	return it->second;
}

const Index::Item& Index::operator[](node_editor::Node* ptr) const {
	auto uiIt = m_uiIndex.find(ptr);
	assert(uiIt != m_uiIndex.end());

	auto it = m_data.find(uiIt->second);
	assert(it != m_data.end());
	return it->second;
}

Index::const_iterator Index::begin() const {
	return m_data.begin();
}

Index::const_iterator Index::end() const {
	return m_data.end();
}
