#pragma once

#include "node.h"

namespace dependency_graph {

template<typename T>
void Node::Port::set(const T& value) {
	// can set only inputs, not outputs
	assert(category() == Attr::kInput);
	// setting a value in the middle of the graph might do
	//   weird things, so lets assert it
	assert(m_parent->inputIsNotConnected(*this));

	// set the value in the data block
	m_parent->set<T>(m_id, value);

	// explicitly setting a value makes it not dirty, but makes everything that
	//   depends on it dirty
	m_parent->markAsDirty(m_id);
	setDirty(false);
}

template<typename T>
const T& Node::Port::get() {
	// can get both inputs and outputs

	// do the computation if needed, to get rid of the dirty flag
	if(m_dirty) {
		if(category() == Attr::kInput)
			m_parent->computeInput(m_id);
		else
			m_parent->computeOutput(m_id);
	}
	// when the computation is done, the port should not be dirty
	assert(!m_dirty);

	// and return the value
	return m_parent->get<T>(m_id);
}

//////////

template<typename T>
const T& Node::get(size_t index) const {
	assert(!port(index).isDirty());
	return m_data.get<T>(index);
}

template<typename T>
void Node::set(size_t index, const T& value) {
	assert(port(index).category() == Attr::kOutput || inputIsNotConnected(port(index)));
	m_data.set<T>(index, value);
}

template<typename T>
void Node::setBlindData(const T& value) {
	// create blind data if they're not present
	if(m_blindData.get() == NULL)
		m_blindData = std::unique_ptr<Datablock::BaseData>(new Datablock::Data<T>());

	// retype
	Datablock::Data<T>& val = dynamic_cast<Datablock::Data<T>&>(*m_blindData);

	// set the value
	if(val.value != value) {
		val.value = value;
	}
}

/// blind per-node data, to be used by the client application
///   to store visual information (e.g., node position, colour...)
template<typename T>
const T& Node::blindData() const {
	// retype and return
	const Datablock::Data<T>& val = dynamic_cast<const Datablock::Data<T>&>(*m_blindData);
	return val.value;
}

}
