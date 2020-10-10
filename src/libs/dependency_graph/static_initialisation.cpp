#include "static_initialisation.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <string>

namespace dependency_graph {

namespace {

typedef std::map<std::string, std::weak_ptr<const std::function<Data()>>> Factories;

/// returns a valid instance - either explicitly created, or implicit
Factories& factories() {
	static std::unique_ptr<Factories> s_factories;
	if(s_factories == nullptr)
		s_factories = std::unique_ptr<Factories>(new Factories());

	return *s_factories;
};

}  // namespace

FactoryHandle StaticInitialisation::registerDataFactory(const std::string& type, std::function<Data()> fn) {
	auto it = factories().find(type);
	if(it != factories().end())
		return FactoryHandle(it->second.lock());

	std::shared_ptr<const std::function<Data()>> ptr(new std::function<Data()>(fn));
	factories().insert(std::make_pair(type, ptr));

	return FactoryHandle(ptr);
}

Data StaticInitialisation::create(const std::string& type) {
	auto it = factories().find(type);

	if(it == factories().end()) {
		std::stringstream err;
		err << "Error instantiating type '" << type << "' - no registered factory found (plugin not loaded?)";

		throw std::runtime_error(err.str());
	}

	return it->second.lock()->operator()();
}

}  // namespace dependency_graph
