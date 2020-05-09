#include "sequence.h"

#include "tools.h"

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
	m_sequence.push_back(Item(frame));
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

const Sequence::Item& Sequence::front() const {
	return m_sequence.front();
}

const Sequence::Item& Sequence::back() const {
	return m_sequence.back();
}

Sequence::Item& Sequence::operator[](std::size_t index) {
	assert(index < m_sequence.size());
	return m_sequence[index];
}

const Sequence::Item& Sequence::operator[](std::size_t index) const {
	assert(index < m_sequence.size());
	return m_sequence[index];
}

bool Sequence::operator == (const Sequence& f) const {
	return m_sequence == f.m_sequence;
}

bool Sequence::operator != (const Sequence& f) const {
	return m_sequence != f.m_sequence;
}

///////////////

Sequence::Item::Item() {
}

Sequence::Item::Item(const cv::Mat& m) : m_mat(m) {
}

Sequence::Item::Meta& Sequence::Item::meta() {
	return m_meta;
}

const Sequence::Item::Meta& Sequence::Item::meta() const {
	return m_meta;
}

bool Sequence::Item::operator == (const Item& i) const {
	return m_mat.ptr() == i->ptr();
}

bool Sequence::Item::operator != (const Item& i) const {
	return m_mat.ptr() != i->ptr();
}

/////////////

std::ostream& operator << (std::ostream& out, const Sequence& f) {
	if(f.empty())
		out << "(empty sequence)" << std::endl;
	else if(f.size() == 1)
		out << "(a sequence with 1 frame, " << opencv::type2str((*f[0]).type()) << ", " << (*f[0]).cols << "x" << (*f[0]).rows << ")";
	else
		out << "(a sequence of " << f.size() << " frames, " << opencv::type2str((*f[0]).type()) << ", " << (*f[0]).cols << "x" << (*f[0]).rows << ")";

	return out;
}

} }
