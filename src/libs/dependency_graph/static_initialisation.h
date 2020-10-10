#pragma once

#include <functional>
#include <map>
#include <memory>

#include "data.h"
#include "factory_handle.h"

namespace dependency_graph {

class StaticInitialisation {
  public:
	static FactoryHandle registerDataFactory(const std::string& type, std::function<Data()> fn);
	static Data create(const std::string& type);
};

}  // namespace dependency_graph
