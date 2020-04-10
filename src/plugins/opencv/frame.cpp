#include "frame.h"

#include "tools.h"

namespace possumwood { namespace opencv {

Frame::Frame(const cv::Mat& data, bool copy) {
	if(!copy)
		m_frame = cv::Mat(data);
	else
		m_frame = cv::Mat(data.clone());
}

Frame Frame::clone() const {
	return Frame(m_frame, true);
}

const cv::Mat& Frame::operator*() const {
	return m_frame;
}

const cv::Mat* Frame::operator->() const {
	return &m_frame;
}

cv::Mat& Frame::operator*() {
	return m_frame;
}

cv::Mat* Frame::operator->() {
	return &m_frame;
}

cv::Size Frame::size() const {
	return cv::Size(m_frame.cols, m_frame.rows);
}

bool Frame::empty() const {
	return m_frame.empty();
}

bool Frame::operator == (const Frame& f) const {
	return m_frame.ptr() == f.m_frame.ptr();
}

bool Frame::operator != (const Frame& f) const {
	return m_frame.ptr() != f.m_frame.ptr();
}

std::ostream& operator << (std::ostream& out, const Frame& f) {
	out << "(" << opencv::type2str((*f).type()) << " frame, " << (*f).cols << "x" << (*f).rows << ")";
	return out;
}

} }
