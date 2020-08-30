#pragma once

#include <actions/traits.h>

#include <iostream>

namespace possumwood {
namespace opencv {

/// A simple typed container for basic EXIF data
class Exif {
  public:
	Exif();
	Exif(float exposure, float f, float iso);

	bool valid() const;

	float exposure() const;
	float f() const;
	float iso() const;

	bool operator==(const Exif& e) const;
	bool operator!=(const Exif& e) const;

  private:
	float m_exposure, m_f, m_iso;
};

std::ostream& operator<<(std::ostream& out, const Exif& f);

}  // namespace opencv

template <>
struct Traits<opencv::Exif> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0.3, 0.3, 0.6}};
	}
};

}  // namespace possumwood
