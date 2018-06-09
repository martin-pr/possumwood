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

const Attr& Metadata::attr(size_t index) const {
	assert(index < m_attrs.size());
	return m_attrs[index];
}

void Metadata::addAttribute(InAttr<void>& in, const std::string& name) {
	assert(!in.isValid());

	in = InAttr<void>(name);
	doAddAttribute(in);

	assert(in.isValid());
}

void Metadata::addAttribute(OutAttr<void>& out, const std::string& name) {
	assert(!out.isValid());

	out = OutAttr<void>(name);
	doAddAttribute(out);

	assert(out.isValid());
}

void Metadata::doAddAttribute(Attr& attr) {
	attr.setOffset(m_attrs.size());

	m_attrs.push_back(attr);
}

std::vector<std::size_t> Metadata::influences(size_t index) const {
	std::vector<std::size_t> result;

	auto i1 = m_influences.left.lower_bound(index);
	auto i2 = m_influences.left.upper_bound(index);
	for(auto i = i1; i != i2; ++i)
		result.push_back(i->second);

	return result;
}

std::vector<std::size_t> Metadata::influencedBy(size_t index) const {
	std::vector<std::size_t> result;

	auto i1 = m_influences.right.lower_bound(index);
	auto i2 = m_influences.right.upper_bound(index);
	for(auto i = i1; i != i2; ++i)
		result.push_back(i->second);

	return result;
}

MetadataHandle::MetadataHandle(std::unique_ptr<Metadata> m) : m_meta(m.release()) {
}

MetadataHandle::MetadataHandle(const Metadata& meta) : m_meta(meta.shared_from_this()) {
}

MetadataHandle::~MetadataHandle() {
}

const Metadata& MetadataHandle::metadata() const {
	assert(m_meta != nullptr);
	return *m_meta;
}

MetadataHandle::operator const Metadata&() const {
	return *m_meta;
}

const Metadata* MetadataHandle::operator->() const {
	return m_meta.get();
}

bool MetadataHandle::operator == (const MetadataHandle& h) const {
	return m_meta == h.m_meta;
}

bool MetadataHandle::operator != (const MetadataHandle& h) const {
	return m_meta != h.m_meta;
}

}
