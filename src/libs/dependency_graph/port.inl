#pragma once

#include "port.h"

#include "node.h"
#include "graph.h"

namespace dependency_graph {

template<typename T>
void Port::set(const T& value) {
	const Data<T> val = dependency_graph::Data<T>(value);
	setData(val);
}

template<typename T>
const T& Port::get() {
	const Data<T>& d = dynamic_cast<const Data<T>&>(getData());
	return d.value;
}

}
