#pragma once

#include <functional>
#include <memory>

namespace dependency_graph {

class Data;

class StaticInitialisation;

class FactoryHandle {
  public:
	FactoryHandle() = default;
	~FactoryHandle() = default;

  private:
	FactoryHandle(std::shared_ptr<const std::function<Data()>> ptr) : m_ptr(ptr) {
	}

	std::shared_ptr<const std::function<Data()>> m_ptr;

	friend class StaticInitialisation;
};

}  // namespace dependency_graph
