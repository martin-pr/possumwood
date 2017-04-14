#include "datablock.h"

#include "metadata.h"
#include "attr.h"

namespace dependency_graph {

Datablock::Datablock(const Metadata& meta) {
	for(std::size_t a=0; a<meta.attributeCount(); ++a)
		m_data.push_back(std::move(meta.attr(a).createData()));
}

const Datablock::BaseData& Datablock::data(size_t index) const {
	assert(m_data.size() > index);
	return *m_data[index];
}


Datablock::BaseData& Datablock::data(size_t index) {
	assert(m_data.size() > index);
	return *m_data[index];
}

}
