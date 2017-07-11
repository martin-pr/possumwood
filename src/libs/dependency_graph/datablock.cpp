#include "datablock.h"

#include "metadata.h"
#include "attr.h"

namespace dependency_graph {

Datablock::Datablock(const Metadata& meta) : m_meta(&meta) {
	for(std::size_t a=0; a<meta.attributeCount(); ++a)
		m_data.push_back(meta.attr(a).createData());
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

}
