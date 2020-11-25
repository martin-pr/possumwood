#include "sequence.h"

#include "tools.h"

namespace possumwood {
namespace opencv {

Sequence::Metadata::Metadata() : type(0), rows(0), cols(0), depth(0), channels(0) {
}

Sequence::Metadata::Metadata(int type_, int rows_, int cols_) : type(type_), rows(rows_), cols(cols_) {
	depth = type & CV_MAT_DEPTH_MASK;
	channels = 1 + (type >> CV_CN_SHIFT);
}

Sequence::MatProxy& Sequence::MatProxy::operator=(cv::Mat&& m) {
	// faking move semantics
	*m_mat = m;
	m = cv::Mat();

	m_seq->updateMeta(m_index, *m_mat);

	return *this;
}

Sequence::MatProxy::MatProxy(const Imath::V2i& index, cv::Mat* m, Sequence* s) : m_index(index), m_mat(m), m_seq(s) {
}

Sequence::MatProxy::operator const cv::Mat &() const {
	return *m_mat;
}

Sequence::Sequence(const Metadata& meta) : m_meta(meta), m_min(0, 0), m_max(0, 0) {
}

Sequence::Sequence(const Sequence& s) : m_sequence(s.m_sequence), m_meta(s.m_meta), m_min(s.m_min), m_max(s.m_max) {
}

Sequence& Sequence::operator=(const Sequence& s) {
	m_sequence = s.m_sequence;
	m_meta = s.m_meta;
	m_min = s.m_min;
	m_max = s.m_max;

	return *this;
}

bool Sequence::empty() const {
	return m_sequence.empty();
}

bool Sequence::hasOneElement() const {
	return m_sequence.size() == 1;
}

const Imath::V2i& Sequence::min() const {
	return m_min;
}

const Imath::V2i& Sequence::max() const {
	return m_max;
}

const Sequence::Metadata& Sequence::meta() const {
	return m_meta;
}

namespace {

static const cv::Mat s_empty;

}

const cv::Mat& Sequence::operator()(int x, int y) const {
	auto it = m_sequence.find(Imath::V2i(x, y));
	if(it == m_sequence.end())
		return s_empty;

	return it->second;
}

Sequence::MatProxy Sequence::operator()(int x, int y) {
	std::lock_guard<std::mutex> guard(m_mutex);

	return MatProxy(Imath::V2i(x, y), &(m_sequence[Imath::V2i(x, y)]), this);
}

const cv::Mat& Sequence::operator[](const Imath::V2i& index) const {
	auto it = m_sequence.find(index);
	if(it == m_sequence.end())
		return s_empty;

	return it->second;
}

Sequence::MatProxy Sequence::operator[](const Imath::V2i& index) {
	std::lock_guard<std::mutex> guard(m_mutex);

	return MatProxy(index, &(m_sequence[index]), this);
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

Sequence::const_iterator Sequence::find(const Imath::V2i& index) const {
	return m_sequence.find(index);
}

void Sequence::updateMeta(const Imath::V2i& index, const cv::Mat& m) {
	// first image
	if(m_meta.channels == 0) {
		m_meta.channels = m.channels();
		m_meta.cols = m.cols;
		m_meta.depth = m.depth();
		m_meta.rows = m.rows;
		m_meta.type = m.type();

		m_min = index;
		m_max = index;
	}
	// images already present - compare + throw
	else {
		if(m_meta.channels != m.channels())
			throw std::runtime_error("Inserting incompatible matrix into a sequence - the sequence has " +
			                         std::to_string(m_meta.channels) + " channels while the matrix has " +
			                         std::to_string(m.channels()));

		if(m_meta.cols != m.cols)
			throw std::runtime_error("Inserting incompatible matrix into a sequence - the sequence has " +
			                         std::to_string(m_meta.cols) + " columns while the matrix has " +
			                         std::to_string(m.cols));

		if(m_meta.depth != m.depth())
			throw std::runtime_error("Inserting incompatible matrix into a sequence - the sequence has " +
			                         std::to_string(m_meta.depth) + " depth while the matrix has " +
			                         std::to_string(m.depth()));

		if(m_meta.rows != m.rows)
			throw std::runtime_error("Inserting incompatible matrix into a sequence - the sequence has " +
			                         std::to_string(m_meta.rows) + " rows while the matrix has " +
			                         std::to_string(m.rows));

		if(m_meta.type != m.type())
			throw std::runtime_error("Inserting incompatible matrix into a sequence - the sequence has " +
			                         opencv::type2str(m_meta.type) + " type while the matrix has " +
			                         opencv::type2str(m.type()));

		m_min.x = std::min(m_min.x, index.x);
		m_max.x = std::max(m_max.x, index.x);
		m_min.y = std::min(m_min.y, index.y);
		m_max.y = std::max(m_max.y, index.y);
	}
}

bool Sequence::operator==(const Sequence& f) const {
	if(m_meta != f.m_meta)
		return false;

	if(m_sequence.size() != f.m_sequence.size())
		return false;

	auto it1 = m_sequence.begin();
	auto it2 = f.begin();

	while(it1 != m_sequence.end()) {
		if(it1->first != it2->first || it1->second.data != it2->second.data)
			return false;

		++it1;
		++it2;
	}

	return true;
}

bool Sequence::operator!=(const Sequence& f) const {
	if(m_meta != f.m_meta)
		return true;

	if(m_sequence.size() != f.m_sequence.size())
		return true;

	auto it1 = m_sequence.begin();
	auto it2 = f.begin();

	while(it1 != m_sequence.end()) {
		if(it1->first != it2->first || it1->second.data != it2->second.data)
			return true;

		++it1;
		++it2;
	}

	return false;
}

bool Sequence::hasMatchingKeys(const Sequence& s1, const Sequence& s2) {
	if(s1.m_sequence.size() != s2.m_sequence.size())
		return false;

	auto it1 = s1.m_sequence.begin();
	auto it2 = s2.m_sequence.begin();
	while(it1 != s1.m_sequence.end()) {
		if(it1->first != it2->first)
			return false;

		++it1;
		++it2;
	}

	return true;
}

bool Sequence::Comparator::operator()(const Imath::V2i& v1, const Imath::V2i& v2) const {
	if(v1.y != v2.y)
		return v1.y < v2.y;
	return v1.x < v2.x;
}

bool Sequence::Metadata::operator==(const Metadata& meta) const {
	return type == meta.type && rows == meta.rows && cols == meta.cols && depth == meta.depth &&
	       channels == meta.channels;
}

bool Sequence::Metadata::operator!=(const Metadata& meta) const {
	return type != meta.type || rows != meta.rows || cols != meta.cols || depth != meta.depth ||
	       channels != meta.channels;
}

/////////////

std::ostream& operator<<(std::ostream& out, const Sequence& seq) {
	if(seq.empty())
		out << "Empty sequence" << std::endl;
	else if(seq.m_sequence.size() == 1)
		out << "A sequence with 1 frame, " << opencv::type2str(seq.meta().type) << ", " << seq.meta().cols << "x"
		    << seq.meta().rows << std::endl;
	else
		out << "A sequence with " << seq.m_sequence.size() << " frames, " << opencv::type2str(seq.meta().type) << ", "
		    << seq.meta().cols << "x" << seq.meta().rows << std::endl;

	return out;
}

}  // namespace opencv
}  // namespace possumwood
