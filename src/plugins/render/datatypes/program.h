#pragma once

#include <memory>

#include <actions/traits.h>

#include <dependency_graph/state.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "shader.h"

namespace possumwood {

/// OpenGL Program abstraction, to allow it to be passed through the node graph as data
class Program : public boost::noncopyable {
	public:
		Program();
		virtual ~Program();

		/// returns the current state of this program
		const dependency_graph::State& state() const;
		/// returns the program ID (not be used in GL commands)
		GLuint id() const;

		void addShader(const Shader& s);

		/// links this program
		void link();

	private:
		dependency_graph::State m_state;

		GLuint m_programId;
		std::vector<GLuint> m_shaderIds;
};

template<>
struct Traits<std::shared_ptr<const Program>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0, 1}};
	}

};

}
