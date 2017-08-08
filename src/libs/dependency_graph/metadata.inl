#pragma once

#include "metadata.h"

#include "data.inl"

namespace dependency_graph {

template<typename T>
void Metadata::addAttribute(InAttr<T>& in, const std::string& name, const T& defaultValue) {
	assert(!in.isValid());

	in = InAttr<T>(name, m_attrs.size(), defaultValue);
	m_attrs.push_back(&in);

	assert(in.isValid());
}

template<typename T>
void Metadata::addAttribute(OutAttr<T>& out, const std::string& name, const T& defaultValue) {
	assert(!out.isValid());

	out = OutAttr<T>(name, m_attrs.size(), defaultValue);
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

template<typename T>
void Metadata::setBlindData(const T& value) {
	// create blind data if they're not present
	if(m_blindData.get() == NULL)
		m_blindData = std::unique_ptr<BaseData>(new Data<T>());

	// retype
	Data<T>& val = dynamic_cast<Data<T>&>(*m_blindData);

	// set the value
	if(val.value != value)
		val.value = value;
}

/// blind per-node data, to be used by the client application
///   to store visual information (e.g., node position, colour...)
template<typename T>
const T& Metadata::blindData() const {
	// retype and return
	assert(m_blindData != NULL);
	assert(m_blindData->type() == unmangledTypeId<T>());
	const Data<T>& val = dynamic_cast<const Data<T>&>(*m_blindData);
	return val.value;
}

}
