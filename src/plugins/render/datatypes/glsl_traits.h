#pragma once

#include <array>

#include <ImathVec.h>
#include <ImathMatrix.h>

namespace possumwood {

template<typename T>
struct GLSLTraits {
};

template<>
struct GLSLTraits<float> {
	constexpr static unsigned width() { return 1; };
	constexpr static GLenum type() { return GL_FLOAT; };
	constexpr static const char* typeString() { return "float"; };
	static void applyUniform(GLint attr, std::size_t size, const float* val) { glUniform1fv(attr, size, val); };

	static void applyUniformVec2(GLint attr, std::size_t size, const float* val) { glUniform2fv(attr, size, val); };
	static void applyUniformVec3(GLint attr, std::size_t size, const float* val) { glUniform3fv(attr, size, val); };
	static void applyUniformMat4(GLint attr, std::size_t size, const float* val) { glUniformMatrix4fv(attr, size, false, val); };
};

template<>
struct GLSLTraits<double> {
	constexpr static unsigned width() { return 1; };
	constexpr static GLenum type() { return GL_DOUBLE; };
	constexpr static const char* typeString() { return "float"; };
	static void applyUniform(GLint attr, std::size_t size, const double* val) { glUniform1dv(attr, size, val); };

	static void applyUniformVec2(GLint attr, std::size_t size, const double* val) { glUniform2dv(attr, size, val); };
	static void applyUniformVec3(GLint attr, std::size_t size, const double* val) { glUniform3dv(attr, size, val); };
	static void applyUniformMat4(GLint attr, std::size_t size, const double* val) {
		// glUniformMatrix4dv(attr, size, false, val);

		// glUniformMatrix4dv not supported on all platforms - need to use 4fv version instead
		float tmp[16];
		for(unsigned a=0;a<16;++a)
			tmp[a] = val[a];

		glUniformMatrix4fv(attr, 1, false, tmp);
	};
};

template<typename T>
struct GLSLTraits<Imath::Vec2<T>> {
	constexpr static unsigned width() { return 2; };
	constexpr static GLenum type() { return GLSLTraits<T>::type(); };
	constexpr static const char* typeString() { return "vec2"; };
	static void applyUniform(GLint attr, std::size_t size, const Imath::V2f* val) { GLSLTraits<T>::applyUniformVec2(attr, size, val[0].getValue()); };
};

template<typename T>
struct GLSLTraits<Imath::Vec3<T>> {
	constexpr static unsigned width() { return 3; };
	constexpr static GLenum type() { return GLSLTraits<T>::type(); };
	constexpr static const char* typeString() { return "vec3"; };
	static void applyUniform(GLint attr, std::size_t size, const Imath::V3f* val) { GLSLTraits<T>::applyUniformVec3(attr, size, val[0].getValue()); };
};

template<typename T>
struct GLSLTraits<Imath::Matrix44<T>> {
	constexpr static unsigned width() { return 16; };
	constexpr static GLenum type() { return GLSLTraits<T>::type(); };
	constexpr static const char* typeString() { return "mat4"; };
	static void applyUniform(GLint attr, std::size_t size, const Imath::Matrix44<T>* val) { GLSLTraits<T>::applyUniformMat4(attr, size, val[0].getValue()); };
};

}
