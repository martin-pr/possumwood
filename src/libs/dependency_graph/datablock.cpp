#include "datablock.h"

#include "attr.h"
#include "data.inl"
#include "metadata.h"
#include "node_base.h"
#include "port.h"
#include "rtti.h"

namespace dependency_graph {

Datablock::Datablock(const MetadataHandle& meta) : m_meta(meta) {
	for(std::size_t a = 0; a < meta.metadata().attributeCount(); ++a)
		m_data.push_back(std::make_pair(meta.metadata().attr(a).createData(), true));
}

const Data& Datablock::data(size_t index) const {
	assert(m_data.size() > index);
	return m_data[index].first;
}

void Datablock::setData(size_t index, const Data& data) {
	assert(m_data.size() > index);

	if(data.type() != m_data[index].first.type() && m_data[index].first.typeinfo() != typeid(void) &&
	   data.typeinfo() != typeid(void))
		throw std::runtime_error("Port value type does not match (new=" + data.type() + ", current=" +
		                         m_data[index].first.type() + ", meta=" + meta()->attr(index).type().name() + ")");

	m_data[index].first = data;
}

bool Datablock::isNull(std::size_t index) const {
	assert(m_data.size() > index);
	return m_data[index].first.typeinfo() == typeid(void);
}

void Datablock::reset(size_t index) {
	assert(m_data.size() > index);
	setData(index, m_meta.metadata().attr(index).createData());
}

const MetadataHandle& Datablock::meta() const {
	return m_meta;
}

bool Datablock::isDirty(std::size_t index) const {
	assert(m_data.size() > index);
	return m_data[index].second;
}

void Datablock::setDirtyFlag(std::size_t index, bool dirty) {
	assert(m_data.size() > index);
	m_data[index].second = dirty;
}

}  // namespace dependency_graph
