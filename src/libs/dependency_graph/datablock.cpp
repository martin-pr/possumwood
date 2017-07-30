#include "datablock.h"

#include "metadata.h"
#include "attr.h"

namespace dependency_graph {

Datablock::Datablock(const Metadata& meta) : m_meta(&meta) {
	for(std::size_t a=0; a<meta.attributeCount(); ++a)
		m_data.push_back(meta.attr(a).createData());
}

Datablock::Datablock(const Datablock& d) {
	for(auto& a : d.m_data)
		m_data.push_back(a->clone());
}

Datablock& Datablock::operator = (const Datablock& d) {
	m_data.clear();

	for(auto& a : d.m_data)
		m_data.push_back(a->clone());

	return *this;
}

const BaseData& Datablock::data(size_t index) const {
	assert(m_data.size() > index);
	return *m_data[index];
}


BaseData& Datablock::data(size_t index) {
	assert(m_data.size() > index);
	return *m_data[index];
}

void Datablock::reset(size_t index) {
	assert(m_data.size() > index);
	m_data[index] = m_meta->attr(index).createData();
}

const Metadata& Datablock::meta() const {
	return *m_meta;
}

}
