#pragma once

#include <array>

#include <ImathVec.h>

namespace possumwood {

template<typename T>
struct VBOTraits {
	typedef T element;
	static constexpr std::size_t width() { return 1; };
};

template<typename T, std::size_t WIDTH>
struct VBOTraits<std::array<T, WIDTH>> {
	typedef T element;
	static constexpr std::size_t width() { return WIDTH; };
};

template<typename T>
struct VBOTraits<Imath::Vec3<T>> {
	typedef T element;
	static constexpr std::size_t width() { return 3; };
};

}
