#pragma once

#include <memory>

#include <actions/traits.h>

#include <dependency_graph/state.h>

#include <GL/glew.h>
#include <GL/gl.h>

namespace possumwood {

/// Generic shader type - not to be used directly, but only via explicit concrete
/// specialisations below. This allows to distinguish between shaders by their C++ type.
class Shader {
	public:
		Shader();

		Shader(GLenum shaderType, const std::string& source);
		virtual ~Shader();

		/// returns the current state of this shader
		const dependency_graph::State& state() const;
		/// returns the shader's ID (not be used in GL commands)
		GLuint id() const;

		/// compiles this shader, and sets the state according to the success or failure
		void compile();

	private:
		struct Pimpl;
		std::shared_ptr<Pimpl> m_pimpl;

		const std::string m_source;
		GLenum m_shaderType;
};

std::ostream& operator <<(std::ostream& out, const Shader& s);

/// Concrete type for a Fragment shader - to allow explicit type checking
class FragmentShader : public Shader {
	public:
		FragmentShader() {}
		FragmentShader(const std::string& source) : Shader(GL_FRAGMENT_SHADER, source) {
		}
};

/// Concrete type for a Vertex shader - to allow explicit type checking
class VertexShader : public Shader {
	public:
		VertexShader() {}
		VertexShader(const std::string& source) : Shader(GL_VERTEX_SHADER, source) {
		}
};

/// Concrete type for a Vertex shader - to allow explicit type checking
class GeometryShader : public Shader {
	public:
		GeometryShader() {}
		GeometryShader(const std::string& source) : Shader(GL_GEOMETRY_SHADER, source) {
		}
};

template<>
struct Traits<FragmentShader> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.6, 0, 0.6}};
	}

};

template<>
struct Traits<VertexShader> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.3, 0, 0.3}};
	}

};

template<>
struct Traits<GeometryShader> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.45, 0, 0.45}};
	}

};

}
