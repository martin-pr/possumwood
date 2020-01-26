#pragma once

#include "port.h"

#include "node.h"
#include "graph.h"

namespace dependency_graph {

template<typename T>
void Port::set(const T& value) {
	const TypedData<T> val = dependency_graph::TypedData<T>(value);
	setData(val);
}

template<typename T>
const T& Port::get() {
	const TypedData<T>& d = dynamic_cast<const TypedData<T>&>(getData());
	return d.get();
}

}
