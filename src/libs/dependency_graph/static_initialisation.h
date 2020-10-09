#pragma once

#include <functional>
#include <map>
#include <memory>

#include "data.h"
#include "factory_handle.h"

namespace dependency_graph {

/// An explicit static data holder, to allow for explicit control over static data destruction order.
/// This is not a well encapsulated class, but as it is quite internal and not part of the main API, I am leaving it
/// like this for now.
class StaticInitialisation {
  public:
	StaticInitialisation();
	~StaticInitialisation();

	static FactoryHandle registerDataFactory(const std::string& type, std::function<Data()> fn);

	static Data create(const std::string& type);

	/// Private implementation structure
	struct Pimpl;

  private:
	std::unique_ptr<Pimpl> m_pimpl;

	StaticInitialisation(const StaticInitialisation&) = delete;
	StaticInitialisation& operator=(const StaticInitialisation&) = delete;
};

}  // namespace dependency_graph
