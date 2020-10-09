#include "static_initialisation.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <string>

namespace dependency_graph {

namespace {
StaticInitialisation::Pimpl* s_currentInstance = nullptr;
}

/// Private implementation - only one instance can exist at any point
struct StaticInitialisation::Pimpl {
	Pimpl() {
		assert(s_currentInstance == nullptr);
		s_currentInstance = this;
	}

	~Pimpl() {
		assert(s_currentInstance == this);
		s_currentInstance = nullptr;
	}

	Pimpl(const Pimpl&) = delete;
	Pimpl& operator=(const Pimpl&) = delete;

	std::map<std::string, std::weak_ptr<const std::function<Data()>>> factories;
};

namespace {
/// static Pimpl instance, if the Static initialisation class is not used explicitly - automagically created
std::unique_ptr<StaticInitialisation::Pimpl> s_pimpl;

/// returns a valid instance - either explicitly created, or implicit
StaticInitialisation::Pimpl& instance() {
	if(s_currentInstance != nullptr)
		return *s_currentInstance;

	s_pimpl = std::unique_ptr<StaticInitialisation::Pimpl>(new StaticInitialisation::Pimpl());
	assert(s_pimpl.get() == s_currentInstance);

	return *s_currentInstance;
};
}  // namespace

StaticInitialisation::StaticInitialisation() {
	if(s_pimpl.get() != nullptr)
		m_pimpl = std::move(s_pimpl);
	else
		m_pimpl = std::unique_ptr<Pimpl>(new Pimpl());

	assert(m_pimpl.get() == s_currentInstance);
}

StaticInitialisation::~StaticInitialisation() {
}

FactoryHandle StaticInitialisation::registerDataFactory(const std::string& type, std::function<Data()> fn) {
	auto it = instance().factories.find(type);
	if(it != instance().factories.end())
		return FactoryHandle(it->second.lock());

	std::shared_ptr<const std::function<Data()>> ptr(new std::function<Data()>(fn));
	instance().factories.insert(std::make_pair(type, ptr));

	return FactoryHandle(ptr);
}

Data StaticInitialisation::create(const std::string& type) {
	auto it = instance().factories.find(type);

	if(it == instance().factories.end()) {
		std::stringstream err;
		err << "Error instantiating type '" << type << "' - no registered factory found (plugin not loaded?)";

		throw std::runtime_error(err.str());
	}

	return it->second.lock()->operator()();
}

}  // namespace dependency_graph
