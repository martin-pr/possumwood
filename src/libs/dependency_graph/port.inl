#pragma once

#include "graph.h"
#include "node.h"
#include "port.h"

namespace dependency_graph {

template <typename T>
void Port::set(const T& value) {
	setData(dependency_graph::Data(value));
}

template <typename T>
void Port::set(T&& value) {
	if(type() == typeid(void))
		throw std::runtime_error("Attempting to directly set a void port");

	setData(dependency_graph::Data(std::move(value)));
}

template <typename T>
const T& Port::get() {
	return getData().get<T>();
}

}  // namespace dependency_graph
