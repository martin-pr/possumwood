#pragma once

#include <type_traits>

#include <dependency_graph/rtti.h>

#include "io.h"

namespace possumwood {

/// defaults traits struct (not implemented)
template<typename T, typename ENABLE = void>
struct Traits;

/// traits for numbers
template<typename T>
struct Traits<T, typename std::enable_if< std::is_arithmetic<T>::value >::type> {
	static void toJson(dependency_graph::io::json& json, const T& f) {
		json = f;
	}

	static void fromJson(const dependency_graph::io::json& json, T& f) {
		f = json.get<T>();
	}

	static IO<T> io;
};

template<typename T>
IO<T> Traits<T, typename std::enable_if< std::is_arithmetic<T>::value >::type>::io(&toJson, &fromJson);

}
