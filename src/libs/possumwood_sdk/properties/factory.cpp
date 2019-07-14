#include "factory.h"

#include <cassert>

#include "property.h"

namespace possumwood { namespace properties {

factories& factories::singleton() {
	static std::unique_ptr<factories> s_factories;
	if(s_factories.get() == NULL)
		s_factories = std::unique_ptr<factories>(new factories());

	return *s_factories;
}

factories::factories() {
}

void factories::add(factory* f) {
	assert(f != NULL);

#ifndef NDEBUG
	auto it = m_factories.find(f->type());
	assert(it == m_factories.end());
#endif

	m_factories[f->type()] = f;
}

void factories::remove(factory* f) {
	assert(f != NULL);

	auto it = m_factories.find(f->type());
	assert(it != m_factories.end());

	m_factories.erase(it);
}

std::unique_ptr<property_base> factories::create(const std::type_index& type) {
	auto it = m_factories.find(type);

	if(it == m_factories.end())
		return std::unique_ptr<property_base>();

	return it->second->create();
}


///////

factory::factory() {
}

factory::~factory() {
}

} }
