#include "exif_sequence.h"

namespace possumwood { namespace opencv {

void ExifSequence::add(const Exif& exif) {
	m_sequence.push_back(exif);
}

bool ExifSequence::empty() const {
	return m_sequence.empty();
}

std::size_t ExifSequence::size() const {
	return m_sequence.size();
}

ExifSequence::const_iterator ExifSequence::begin() const {
	return m_sequence.begin();
}

ExifSequence::const_iterator ExifSequence::end() const {
	return m_sequence.end();
}

bool ExifSequence::operator == (const ExifSequence& f) const {
	return m_sequence == f.m_sequence;
}

bool ExifSequence::operator != (const ExifSequence& f) const {
	return m_sequence != f.m_sequence;
}

std::ostream& operator << (std::ostream& out, const ExifSequence& f) {
	out << "Exif sequence with " << f.size() << " item(s)";

	return out;
}

} }
