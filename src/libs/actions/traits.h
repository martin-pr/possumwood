#pragma once

#include <type_traits>

#include <dependency_graph/rtti.h>

#include "actions/io.h"

namespace possumwood {

/// defaults traits struct (not implemented)
template<typename T, typename ENABLE = void>
struct Traits;

/// traits for numbers
template<typename T>
struct Traits<T, typename std::enable_if< std::is_arithmetic<T>::value >::type> {
	static void toJson(possumwood::io::json& json, const T& f) {
		json = f;
	}

	static void fromJson(const possumwood::io::json& json, T& f) {
		f = json.get<T>();
	}

	static IO<T> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.2*sizeof(T), 0.2*sizeof(T), 0.2*sizeof(T)}};
	}
};

template<typename T>
IO<T> Traits<T, typename std::enable_if< std::is_arithmetic<T>::value >::type>::io(&toJson, &fromJson);

/// traits for strings
template<>
struct Traits<std::string> {
	static IO<std::string> io;
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 1}};
	}
};

/// traits for untyped
template<>
struct Traits<void> {
	// static IO<std::string> io;
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0, 0}};
	}
};

}
