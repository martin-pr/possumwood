#pragma once

#include "port.h"

#include "node.h"
#include "graph.h"

namespace dependency_graph {

template<typename T>
void Port::set(const T& value) {
	setData(dependency_graph::Data(value));
}

template<typename T>
const T& Port::get() {
	return getData().get<T>();
}

}
