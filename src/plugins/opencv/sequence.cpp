#include "sequence.h"

namespace possumwood { namespace opencv {

Sequence::Sequence(std::size_t size) : m_sequence(size) {
}

Sequence Sequence::clone() const {
	Sequence result;

	for(auto& f : m_sequence)
		result.add(f->clone());

	return result;
}

void Sequence::add(const cv::Mat& frame) {
	m_sequence.push_back(Frame((frame)));
}

bool Sequence::isValid() const {
	for(auto& f : m_sequence)
		if(f->rows != m_sequence.front()->rows || f->cols != m_sequence.front()->cols || f->type() != m_sequence.front()->type())
			return false;
	return true;
}

bool Sequence::empty() const {
	return m_sequence.empty();
}

std::size_t Sequence::size() const {
	return m_sequence.size();
}

int Sequence::type() const {
	if(m_sequence.empty())
		return 0;
	return m_sequence.front()->type();
}

int Sequence::rows() const {
	if(m_sequence.empty())
		return 0;
	return m_sequence.front()->rows;
}

int Sequence::cols() const {
	if(m_sequence.empty())
		return 0;
	return m_sequence.front()->cols;
}

Sequence::iterator Sequence::begin() {
	return m_sequence.begin();
}

Sequence::iterator Sequence::end() {
	return m_sequence.end();
}

Sequence::const_iterator Sequence::begin() const {
	return m_sequence.begin();
}

Sequence::const_iterator Sequence::end() const {
	return m_sequence.end();
}

const Frame& Sequence::front() const {
	return m_sequence.front();
}

const Frame& Sequence::back() const {
	return m_sequence.back();
}

Frame& Sequence::operator[](std::size_t index) {
	assert(index < m_sequence.size());
	return m_sequence[index];
}

const Frame& Sequence::operator[](std::size_t index) const {
	assert(index < m_sequence.size());
	return m_sequence[index];
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
