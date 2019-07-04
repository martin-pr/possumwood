#include "sequence.h"

namespace possumwood { namespace opencv {

void Sequence::add(const cv::Mat& frame) {
	m_Sequence.push_back(std::shared_ptr<const Frame>(new Frame((frame))));
}

bool Sequence::empty() const {
	return m_Sequence.empty();
}

std::size_t Sequence::size() const {
	return m_Sequence.size();
}

Sequence::const_iterator Sequence::begin() const {
	return m_Sequence.begin();
}

Sequence::const_iterator Sequence::end() const {
	return m_Sequence.end();
}

bool Sequence::operator == (const Sequence& f) const {
	return m_Sequence == f.m_Sequence;
}

bool Sequence::operator != (const Sequence& f) const {
	return m_Sequence != f.m_Sequence;
}

std::ostream& operator << (std::ostream& out, const Sequence& f) {
	out << "(" << f.size() << " Sequence)";
	return out;
}

} }
