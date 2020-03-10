#include "sequence.h"

namespace possumwood { namespace opencv {

void Sequence::add(const cv::Mat& frame) {
	m_sequence.push_back(Frame((frame)));
}

bool Sequence::empty() const {
	return m_sequence.empty();
}

std::size_t Sequence::size() const {
	return m_sequence.size();
}

Sequence::const_iterator Sequence::begin() const {
	return m_sequence.begin();
}

Sequence::const_iterator Sequence::end() const {
	return m_sequence.end();
}

bool Sequence::operator == (const Sequence& f) const {
	return m_sequence == f.m_sequence;
}

bool Sequence::operator != (const Sequence& f) const {
	return m_sequence != f.m_sequence;
}

std::ostream& operator << (std::ostream& out, const Sequence& f) {
	out << "(" << f.size() << " Sequence)";
	return out;
}

} }
