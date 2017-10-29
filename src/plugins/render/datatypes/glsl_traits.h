#pragma once

#include <array>

#include <ImathVec.h>

namespace possumwood {

template<typename T>
struct GLSLTraits {
};

template<>
struct GLSLTraits<float> {
	constexpr static unsigned width() { return 1; };
	constexpr static GLenum type() { return GL_FLOAT; };
	static void applyUniform(GLint attr, const float& val) { glUniform1f(attr, val); };
};

template<>
struct GLSLTraits<double> {
	constexpr static unsigned width() { return 1; };
	constexpr static GLenum type() { return GL_DOUBLE; };
	static void applyUniform(GLint attr, const double& val) { glUniform1d(attr, val); };
};

template<>
struct GLSLTraits<Imath::V2f> {
	constexpr static unsigned width() { return 2; };
	constexpr static GLenum type() { return GL_FLOAT; };
	static void applyUniform(GLint attr, const Imath::V2f& val) { glUniform2fv(attr, 1, val.getValue()); };
};

template<>
struct GLSLTraits<Imath::V2d> {
	constexpr static unsigned width() { return 2; };
	constexpr static GLenum type() { return GL_DOUBLE; };
	static void applyUniform(GLint attr, const Imath::V2d& val) { glUniform2dv(attr, 1, val.getValue()); };
};

template<>
struct GLSLTraits<Imath::V3f> {
	constexpr static unsigned width() { return 3; };
	constexpr static GLenum type() { return GL_FLOAT; };
	static void applyUniform(GLint attr, const Imath::V3f& val) { glUniform3fv(attr, 1, val.getValue()); };
};

template<>
struct GLSLTraits<Imath::V3d> {
	constexpr static unsigned width() { return 3; };
	constexpr static GLenum type() { return GL_DOUBLE; };
	static void applyUniform(GLint attr, const Imath::V3d& val) { glUniform3dv(attr, 1, val.getValue()); };
};

template<>
struct GLSLTraits<std::array<float, 16>> {
	constexpr static unsigned width() { return 16; };
	constexpr static GLenum type() { return GL_FLOAT; };
	static void applyUniform(GLint attr, const std::array<float, 16>& val) { glUniformMatrix4fv(attr, 1, false, val.data()); };
};

template<>
struct GLSLTraits<std::array<double, 16>> {
	constexpr static unsigned width() { return 16; };
	constexpr static GLenum type() { return GL_DOUBLE; };
	static void applyUniform(GLint attr, const std::array<double, 16>& val) {
		// glUniformMatrix4dv not supported on all platforms - need to use 4fv version instead
		float tmp[16];
		for(unsigned a=0;a<16;++a)
			tmp[a] = val[a];

		glUniformMatrix4fv(attr, 1, false, tmp);
	};
};

}
