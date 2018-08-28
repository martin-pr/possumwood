#include "generic_base.inl"

namespace possumwood {
namespace polymesh {

GenericBase::GenericBase() : m_handles(this), m_size(0) {
}

GenericBase::~GenericBase() {
}

GenericBase::GenericBase(const GenericBase& gb) : m_handles(this, gb.handles().m_handles), m_size(gb.m_size) {
	for(auto& c : gb.m_data)
		m_data.push_back(c->clone());
}

GenericBase& GenericBase::operator = (const GenericBase& gb) {
	m_handles = Handles(this, gb.handles().m_handles);

	m_data.clear();
	for(auto& c : gb.m_data)
		m_data.push_back(c->clone());

	m_size = gb.m_size;

	return *this;
}

bool GenericBase::operator ==(const GenericBase& b) const {
	if(m_handles != b.m_handles)
		return false;

	if(m_size != b.m_size)
		return false;

	assert(m_data.size() == b.m_data.size());

	auto it1 = m_data.begin();
	auto it2 = b.m_data.begin();

	while(it1 != m_data.end()) {
		if(not (**it1).isEqual(**it2))
			return false;

		++it1;
		++it2;
	}

	return true;
}
bool GenericBase::operator !=(const GenericBase& b) const {
	if(m_handles != b.m_handles)
		return true;
	assert(m_data.size() == b.m_data.size());

	if(m_size != b.m_size)
		return true;

	auto it1 = m_data.begin();
	auto it2 = b.m_data.begin();

	while(it1 != m_data.end()) {
		if(not (**it1).isEqual(**it2))
			return true;

		++it1;
		++it2;
	}

	return false;
}

///////////

GenericBase::Handle::Handle(const std::string& n, const std::type_index& type, std::size_t index) : m_name(n), m_type(type), m_index(index) {
}

const std::string& GenericBase::Handle::name() const {
	return m_name;
}

const std::type_index& GenericBase::Handle::type() const {
	return m_type;
}

bool GenericBase::Handle::operator ==(const Handle& b) const {
	return m_name == b.m_name && m_type == b.m_type;
}

bool GenericBase::Handle::operator !=(const Handle& b) const {
	return m_name != b.m_name || m_type != b.m_type;
}

//////////

GenericBase::Handles::Handles(GenericBase* parent, const std::set<Handle, Compare>& h) : m_handles(h), m_parent(parent) {
}

bool GenericBase::Handles::empty() const {
	return m_handles.empty();
}

std::size_t GenericBase::Handles::size() const {
	return m_handles.size();
}

const GenericBase::Handle& GenericBase::Handles::operator[](const std::string& name) const {
	auto it = m_handles.find(name);
	assert(it != m_handles.end());
	return *it;
}

GenericBase::Handles::const_iterator GenericBase::Handles::begin() const {
	return m_handles.begin();
}

GenericBase::Handles::const_iterator GenericBase::Handles::end() const {
	return m_handles.end();
}

GenericBase::Handles::const_iterator GenericBase::Handles::find(const std::string& name) const {
	return m_handles.find(name);
}

void GenericBase::Handles::clear() {
	m_handles.clear();
}

bool GenericBase::Handles::operator ==(const Handles& b) const {
	return m_handles == b.m_handles;
}

bool GenericBase::Handles::operator !=(const Handles& b) const {
	return m_handles != b.m_handles;
}

////

GenericBase::Handles& GenericBase::handles() {
	return m_handles;
}

const GenericBase::Handles& GenericBase::handles() const {
	return m_handles;
}

bool GenericBase::empty() const {
	return m_data.empty() || m_data[0]->empty();
}

std::size_t GenericBase::size() const {
	return m_size;
}

void GenericBase::clear() {
	m_data.clear();
	m_handles.clear();
	m_size = 0;
}

void GenericBase::resize(std::size_t size) {
	for(auto& c : m_data)
		c->resize(size);
	m_size = size;
}

std::size_t GenericBase::add() {
	++m_size;

	for(auto& c : m_data)
		c->add();

	return m_size - 1;
}

}
}
