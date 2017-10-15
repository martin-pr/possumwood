#pragma once

#include <ImathVec.h>

namespace possumwood {

template<typename T>
struct VBOTraits {
};

template<>
struct VBOTraits<float> {
	constexpr static unsigned width() { return 1; };
	constexpr static GLenum type() { return GL_FLOAT; };
};

template<>
struct VBOTraits<double> {
	constexpr static unsigned width() { return 1; };
	constexpr static GLenum type() { return GL_DOUBLE; };
};

template<>
struct VBOTraits<Imath::V3f> {
	constexpr static unsigned width() { return 3; };
	constexpr static GLenum type() { return GL_FLOAT; };
};

template<>
struct VBOTraits<Imath::V3d> {
	constexpr static unsigned width() { return 3; };
	constexpr static GLenum type() { return GL_DOUBLE; };
};

}
