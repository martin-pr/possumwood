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

Sequence::Item& Sequence::add(const cv::Mat& frame, const Item::Meta& meta) {
	// check for consistency
	if(!m_sequence.empty())
		if(frame.rows != m_sequence.front()->rows || frame.cols != m_sequence.front()->cols || frame.type() != m_sequence.front()->type())
			throw std::runtime_error("Adding an inconsistent frame to a sequence!"); // TODO: more details

	m_sequence.push_back(Item(frame, meta));
	return m_sequence.back();
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

int Sequence::depth() const {
	if(m_sequence.empty())
		return 0;
	return m_sequence.front()->depth();
}

int Sequence::channels() const {
	if(m_sequence.empty())
		return 0;
	return m_sequence.front()->channels();
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

Sequence::Item::Item(const cv::Mat& m, const Meta& meta) : m_mat(m), m_meta(meta) {
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

bool Sequence::Item::Meta::empty() const {
	return m_meta.empty();
}

float Sequence::Item::Meta::operator[](const std::string& key) const {
	auto it = m_meta.find(key);
	if(it != m_meta.end())
		return it->second;
	return 0.0f;
}

float& Sequence::Item::Meta::operator[](const std::string& key) {
	return m_meta[key];
}

Sequence::Item::Meta::const_iterator Sequence::Item::Meta::begin() const {
	return m_meta.begin();
}

Sequence::Item::Meta::const_iterator Sequence::Item::Meta::end() const {
	return m_meta.end();
}

Sequence::Item::Meta Sequence::Item::Meta::merge(const Meta& m1, const Meta& m2) {
	Sequence::Item::Meta result = m2;
	for(auto& m : m1)
		result[m.first] = m.second;
	return result;
}

/////////////

std::ostream& operator << (std::ostream& out, const Sequence& seq) {
	if(seq.empty())
		out << "Empty sequence" << std::endl;
	else if(seq.size() == 1)
		out << "A sequence with 1 frame, " << opencv::type2str((*seq[0]).type()) << ", " << (*seq[0]).cols << "x" << (*seq[0]).rows << std::endl;
	else
		out << "A sequence of " << seq.size() << " frames, " << opencv::type2str((*seq[0]).type()) << ", " << (*seq[0]).cols << "x" << (*seq[0]).rows << std::endl;

	unsigned ctr = 0;
	for(auto& f : seq) {
		out << "  [" << ctr << "] ->";
		if(f.meta().empty())
			out << " no metadata" << std::endl;
		else {
			for(auto& m : f.meta())
				out << " " << m.first << "=" << m.second;
			out << std::endl;
		}

		++ctr;
	}

	return out;
}

} }
