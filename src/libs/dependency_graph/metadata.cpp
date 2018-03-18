#include "metadata.h"

#include <cassert>

#include <boost/iterator/indirect_iterator.hpp>

#include "attr.h"

namespace dependency_graph {

Metadata::Metadata(const std::string& nodeType) : m_type(nodeType) {
}

Metadata::~Metadata() {
}

bool Metadata::isValid() const {
	return !m_attrs.empty() && m_compute;
}

const std::string& Metadata::type() const {
	return m_type;
}

void Metadata::setCompute(std::function<State(Values&)> compute) {
	m_compute = compute;
}

size_t Metadata::attributeCount() const {
	return m_attrs.size();
}

Attr& Metadata::attr(size_t index) {
	assert(index < m_attrs.size());
	return m_attrs[index];
}

const Attr& Metadata::attr(size_t index) const {
	assert(index < m_attrs.size());
	return m_attrs[index];
}

void Metadata::doAddAttribute(Attr& attr) {
	m_attrs.push_back(attr);
}

std::vector<std::reference_wrapper<const Attr>> Metadata::influences(size_t index) const {
	std::vector<std::reference_wrapper<const Attr>> result;

	auto i1 = m_influences.left.lower_bound(index);
	auto i2 = m_influences.left.upper_bound(index);
	for(auto i = i1; i != i2; ++i)
		result.push_back(attr(i->second));

	return result;
}

std::vector<std::reference_wrapper<const Attr>> Metadata::influencedBy(size_t index) const {
	std::vector<std::reference_wrapper<const Attr>> result;

	auto i1 = m_influences.right.lower_bound(index);
	auto i2 = m_influences.right.upper_bound(index);
	for(auto i = i1; i != i2; ++i)
		result.push_back(attr(i->second));

	return result;
}

MetadataHandle::MetadataHandle(std::unique_ptr<Metadata> m) : m_meta(m.release()) {
}

MetadataHandle::~MetadataHandle() {
}

const Metadata& MetadataHandle::metadata() const {
	assert(m_meta != nullptr);
	return *m_meta;
}

bool MetadataHandle::operator == (const MetadataHandle& h) const {
	return m_meta == h.m_meta;
}

bool MetadataHandle::operator != (const MetadataHandle& h) const {
	return m_meta != h.m_meta;
}

}
