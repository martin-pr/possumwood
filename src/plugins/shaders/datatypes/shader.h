#pragma once

#include <memory>

#include <possumwood_sdk/traits.h>

#include <dependency_graph/state.h>

#include <GL/glew.h>
#include <GL/gl.h>

namespace possumwood {

class Shader : public boost::noncopyable {
	public:
		Shader(GLenum shaderType);
		virtual ~Shader();

		/// returns the current state of this shader
		const dependency_graph::State& state() const;
		/// returns the shader's ID (not be used in GL commands)
		GLuint id() const;

		/// compiles this shader, and sets the state according to the success or failure
		void compile(const std::string& source);

	private:
		dependency_graph::State m_state;

		GLuint m_shaderId;
};

template<>
struct Traits<std::shared_ptr<const Shader>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.5, 0, 0.5}};
	}

};

}
