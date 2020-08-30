#include "exif.h"

namespace possumwood {
namespace opencv {

Exif::Exif() : m_exposure(0.0f), m_f(0.0f), m_iso(0.0f) {
}

Exif::Exif(float exposure, float f, float iso) : m_exposure(exposure), m_f(f), m_iso(iso) {
}

bool Exif::valid() const {
	return m_exposure != 0.0f;
}

float Exif::exposure() const {
	return m_exposure;
}

float Exif::f() const {
	return m_f;
}

float Exif::iso() const {
	return m_iso;
}

bool Exif::operator==(const Exif& e) const {
	return m_exposure == e.m_exposure && m_f == e.m_f && m_iso == e.m_iso;
}

bool Exif::operator!=(const Exif& e) const {
	return m_exposure != e.m_exposure || m_f != e.m_f || m_iso != e.m_iso;
}

std::ostream& operator<<(std::ostream& out, const Exif& f) {
	out << "exposure=" << f.exposure() << "  F=" << f.f() << "  iso=" << f.iso();

	return out;
}

}  // namespace opencv
}  // namespace possumwood
