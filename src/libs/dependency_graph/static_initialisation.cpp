#include "static_initialisation.h"

#include <cassert>
#include <algorithm>
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
	Pimpl& operator = (const Pimpl&) = delete;

	std::map<std::string, std::function<Data()>> factories;
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
}

StaticInitialisation::StaticInitialisation() {
	if(s_pimpl.get() != nullptr)
		m_pimpl = std::move(s_pimpl);
	else
		m_pimpl = std::unique_ptr<Pimpl>(new Pimpl());

	assert(m_pimpl.get() == s_currentInstance);
}

StaticInitialisation::~StaticInitialisation() {
}

void StaticInitialisation::registerDataFactory(const std::string& type, std::function<Data()> fn) {
	instance().factories.insert(std::make_pair(type, fn));
}

const std::map<std::string, std::function<Data()>>& StaticInitialisation::dataFactories() {
	return instance().factories;
}


}
