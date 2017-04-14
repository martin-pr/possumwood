#pragma once

#include "metadata.h"

namespace dependency_graph {

template<typename T>
void Metadata::addAttribute(InAttr<T>& in, const std::string& name) {
	assert(!in.isValid());

	in = InAttr<T>(name, m_attrs.size());
	m_attrs.push_back(&in);

	assert(in.isValid());
}

template<typename T>
void Metadata::addAttribute(OutAttr<T>& out, const std::string& name) {
	assert(!out.isValid());

	out = OutAttr<T>(name, m_attrs.size());
	m_attrs.push_back(&out);

	assert(out.isValid());
}

template<typename T, typename U>
void Metadata::addInfluence(const InAttr<T>& in, const OutAttr<U>& out) {
	assert(in.isValid() && out.isValid());
	assert(std::find(m_attrs.begin(), m_attrs.end(), &in) != m_attrs.end());
	assert(std::find(m_attrs.begin(), m_attrs.end(), &out) != m_attrs.end());

	m_influences.left.insert(std::make_pair(in.offset(), out.offset()));
}

template<typename T>
std::vector<std::reference_wrapper<const Attr>> Metadata::influences(const InAttr<T>& in) const {
	assert(in.isValid());
	return influences(in.offset());
}

template<typename T>
std::vector<std::reference_wrapper<const Attr>> Metadata::influencedBy(const OutAttr<T>& out) const {
	assert(out.isValid());
	return influencedBy(out.offset());
}

}
