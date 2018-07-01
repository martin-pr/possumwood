#pragma once

#include <possumwood_sdk/traits.h>

namespace possumwood {

/// Stores a list of OpenGL parameters that should be applied before a draw call
class GLParameters {
  public:
	class ScopedState : public boost::noncopyable {
	  public:
		~ScopedState();

	  private:
		ScopedState(float lineWidth);

		float m_lineWidth;

		friend class GLParameters;
	};

	GLParameters();

	/// Applies current set of parameters. Returns a scoped object that un-applies the
	/// parameters on destruction.
	std::unique_ptr<ScopedState> apply() const;

	void setLineWidth(float w);
	float lineWidth() const;

	bool operator==(const GLParameters& p) const;
	bool operator!=(const GLParameters& p) const;

  private:
	float m_lineWidth;
};

template <>
struct Traits<GLParameters> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.25, 1}};
	}
};

std::ostream& operator << (std::ostream& out, const GLParameters& p);

}
