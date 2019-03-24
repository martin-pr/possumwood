#include "metadata_register.h"

namespace dependency_graph {

bool MetadataRegister::Compare::operator()(const MetadataHandle& h1, const MetadataHandle& h2) const {
	return h1.metadata().type() < h2.metadata().type();
}

bool MetadataRegister::Compare::operator()(const std::string& h1, const MetadataHandle& h2) const {
	return h1 < h2.metadata().type();
}

bool MetadataRegister::Compare::operator()(const MetadataHandle& h1, const std::string& h2) const {
	return h1.metadata().type() < h2;
}

//////////

MetadataRegister& MetadataRegister::singleton() {
	static std::unique_ptr<MetadataRegister> s_register;
	if(s_register == nullptr)
		s_register = std::unique_ptr<MetadataRegister>(new MetadataRegister());

	return *s_register;
}

MetadataRegister::MetadataRegister() {
}

void MetadataRegister::add(const MetadataHandle& handle) {
	if(m_handles.find(handle.metadata().type()) == m_handles.end())
		m_handles.insert(handle);
	else {
		std::stringstream ss;
		ss << "Node type " << handle->type() << " already registered! Loading a plugin twice would lead to problems. Let's not.";

		throw std::runtime_error(ss.str().c_str());
	}

}

void MetadataRegister::remove(const MetadataHandle& handle) {
	auto it = m_handles.find(handle.metadata().type());
	if(it != m_handles.end())
		m_handles.erase(it);
}

const MetadataHandle& MetadataRegister::operator[](const std::string& nodeName) const {
	auto it = m_handles.find(nodeName);

	if(it == m_handles.end())
		throw std::runtime_error("Node of type " + nodeName + " is not registered - missing a plugin?");

	return *it;
}

MetadataRegister::const_iterator MetadataRegister::begin() const {
	return m_handles.begin();
}

MetadataRegister::const_iterator MetadataRegister::end() const {
	return m_handles.end();
}

}
