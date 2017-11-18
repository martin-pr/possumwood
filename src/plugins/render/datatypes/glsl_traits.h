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
	constexpr static const char* typeString() { return "float"; };
	static void applyUniform(GLint attr, const float& val) { glUniform1f(attr, val); };
};

template<>
struct GLSLTraits<double> {
	constexpr static unsigned width() { return 1; };
	constexpr static GLenum type() { return GL_DOUBLE; };
	constexpr static const char* typeString() { return "float"; };
	static void applyUniform(GLint attr, const double& val) { glUniform1d(attr, val); };
};

template<>
struct GLSLTraits<Imath::V2f> {
	constexpr static unsigned width() { return 2; };
	constexpr static GLenum type() { return GL_FLOAT; };
	constexpr static const char* typeString() { return "vec2"; };
	static void applyUniform(GLint attr, const Imath::V2f& val) { glUniform2fv(attr, 1, val.getValue()); };
};

template<>
struct GLSLTraits<Imath::V2d> {
	constexpr static unsigned width() { return 2; };
	constexpr static GLenum type() { return GL_DOUBLE; };
	constexpr static const char* typeString() { return "vec2"; };
	static void applyUniform(GLint attr, const Imath::V2d& val) { glUniform2dv(attr, 1, val.getValue()); };
};

template<>
struct GLSLTraits<Imath::V3f> {
	constexpr static unsigned width() { return 3; };
	constexpr static GLenum type() { return GL_FLOAT; };
	constexpr static const char* typeString() { return "vec3"; };
	static void applyUniform(GLint attr, const Imath::V3f& val) { glUniform3fv(attr, 1, val.getValue()); };
};

template<>
struct GLSLTraits<Imath::V3d> {
	constexpr static unsigned width() { return 3; };
	constexpr static GLenum type() { return GL_DOUBLE; };
	constexpr static const char* typeString() { return "vec3"; };
	static void applyUniform(GLint attr, const Imath::V3d& val) { glUniform3dv(attr, 1, val.getValue()); };
};

template<>
struct GLSLTraits<std::array<float, 16>> {
	constexpr static unsigned width() { return 16; };
	constexpr static GLenum type() { return GL_FLOAT; };
	constexpr static const char* typeString() { return "mat4"; };
	static void applyUniform(GLint attr, const std::array<float, 16>& val) { glUniformMatrix4fv(attr, 1, false, val.data()); };
};

template<>
struct GLSLTraits<std::array<double, 16>> {
	constexpr static unsigned width() { return 16; };
	constexpr static GLenum type() { return GL_DOUBLE; };
	constexpr static const char* typeString() { return "mat4"; };
	static void applyUniform(GLint attr, const std::array<double, 16>& val) {
		// glUniformMatrix4dv not supported on all platforms - need to use 4fv version instead
		float tmp[16];
		for(unsigned a=0;a<16;++a)
			tmp[a] = val[a];

		glUniformMatrix4fv(attr, 1, false, tmp);
	};
};

namespace {
	template<std::size_t W>
	struct Apply {
	};

	template<>
	struct Apply<1> {
		static void doApplyUniform(GLint attr, const std::array<float, 1>& val) {
			glUniform1f(attr, val[0]);
		}

		static constexpr const char* typeString() {
			return "float";
		}
	};

	template<>
	struct Apply<2> {
		static void doApplyUniform(GLint attr, const std::array<float, 2>& val) {
			glUniform2f(attr, val[0], val[1]);
		}

		static constexpr const char* typeString() {
			return "vec2";
		}
	};

	template<>
	struct Apply<3> {
		static void doApplyUniform(GLint attr, const std::array<float, 3>& val) {
			glUniform3f(attr, val[0], val[1], val[2]);
		}

		static constexpr const char* typeString() {
			return "vec3";
		}
	};

	template<>
	struct Apply<4> {
		static void doApplyUniform(GLint attr, const std::array<float, 4>& val) {
			glUniform4f(attr, val[0], val[1], val[2], val[3]);
		}

		static constexpr const char* typeString() {
			return "vec4";
		}
	};
}

template<std::size_t WIDTH>
struct GLSLTraits<std::array<float, WIDTH>> {
	constexpr static unsigned width() { return WIDTH; };
	constexpr static GLenum type() { return GL_FLOAT; };
	constexpr static const char* typeString() { return Apply<WIDTH>::typeString(); };
	static void applyUniform(GLint attr, const float& val) { Apply<WIDTH>::doApplyUniform(attr, val); };
};


}
