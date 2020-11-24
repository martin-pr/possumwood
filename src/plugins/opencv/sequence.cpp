#include "sequence.h"

#include "tools.h"

namespace possumwood {
namespace opencv {

struct Metadata {
	int type;      // OpenCV type, e.g., CV_8UC1 or CV_32FC3
	int rows;      // number of rows of all images in this sequence
	int cols;      // number of columns of all images in this sequence
	int depth;     // OpenCV depth, e.g., CV_8U or CV_32F
	int channels;  // number of channels (e.g., 1 for mono, 3 for rgb)
};

Sequence::MatProxy& Sequence::MatProxy::operator=(cv::Mat&& m) {
	// faking move semantics
	*m_mat = m;
	m = cv::Mat();

	m_seq->updateMeta(*m_mat);

	return *this;
}

Sequence::MatProxy::MatProxy(cv::Mat* m, Sequence* s) : m_mat(m), m_seq(s) {
}

Sequence::Sequence(std::size_t size) : m_sequence(size) {
}

void Sequence::add(cv::Mat&& frame) {
	// faking move semantics
	m_sequence.push_back(frame);
	frame = cv::Mat();
}

bool Sequence::empty() const {
	return m_sequence.empty();
}

std::size_t Sequence::size() const {
	return m_sequence.size();
}

const Sequence::Metadata& Sequence::meta() const {
	return m_meta;
}

const cv::Mat& Sequence::operator()(std::size_t index) const {
	assert(index < m_sequence.size());
	return m_sequence[index];
}

Sequence::MatProxy Sequence::operator()(std::size_t index) {
	assert(index < m_sequence.size());
	return MatProxy(&m_sequence[index], this);
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

void Sequence::updateMeta(const cv::Mat& m) {
	// first image
	if(m_meta.channels == 0) {
		m_meta.channels = m.channels();
		m_meta.cols = m.cols;
		m_meta.depth = m.depth();
		m_meta.rows = m.rows;
		m_meta.type = m.type();
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
		if(it1->data != it2->data)
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
		if(it1->data != it2->data)
			return true;

		++it1;
		++it2;
	}

	return false;
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
	else if(seq.size() == 1)
		out << "A sequence with 1 frame, " << opencv::type2str(seq.meta().type) << ", " << seq.meta().cols << "x"
		    << seq.meta().rows << std::endl;
	else
		out << "A sequence with " << seq.size() << " frames, " << opencv::type2str(seq.meta().type) << ", "
		    << seq.meta().cols << "x" << seq.meta().rows << std::endl;

	return out;
}

}  // namespace opencv
}  // namespace possumwood
