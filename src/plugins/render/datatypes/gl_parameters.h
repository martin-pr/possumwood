#pragma once

#include <possumwood_sdk/traits.h>

namespace possumwood {

/// Stores a list of OpenGL parameters that should be applied before a draw call
class GLParameters {
  public:
	GLParameters();

	/// Applies current set of parameters. glPushAttrib(GL_ALL) should be performed before
	/// this call.
	void apply() const;

	void setLineWidth(float w);
	float lineWidth() const;

	bool operator==(const GLParameters& p) const;
	bool operator!=(const GLParameters& p) const;

  private:
	float m_lineWidth;
};

template<>
struct Traits<GLParameters> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.25, 1}};
	}

};

}
