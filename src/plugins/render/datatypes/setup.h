#pragma once

#include <iostream>

#include <GL/glew.h>

#include <actions/traits.h>

namespace possumwood {

class GLSetup {
  public:
	enum Culling { kNone, kCCW, kCW };

	GLSetup();

	void setFaceCulling(Culling culling);
	Culling faceCulling() const;

	class ScopedGLSetup {
	  public:
		~ScopedGLSetup();

	  private:
		ScopedGLSetup();

		GLboolean m_cullFaceEnabled;
		GLint m_cullFace, m_frontFace;

		friend class GLSetup;
	};

	/// A RAII class for OpenGL setup - unapplies itself on destruction
	std::unique_ptr<ScopedGLSetup> apply() const;

  private:
	Culling m_faceCulling;
};

std::ostream& operator<<(std::ostream& out, const GLSetup& setup);

template <>
struct Traits<GLSetup> {
	static IO<GLSetup> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.1, 0.1, 0.1}};
	}
};

}  // namespace possumwood
