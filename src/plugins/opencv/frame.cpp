#include "frame.h"

#include "tools.h"

namespace possumwood { namespace opencv {

Frame::Frame(const cv::Mat& data) : m_frame(new cv::Mat(data)) {
}

const cv::Mat& Frame::operator*() const {
	return *m_frame;
}

bool Frame::operator == (const Frame& f) const {
	return m_frame == f.m_frame;
}

bool Frame::operator != (const Frame& f) const {
	return m_frame != f.m_frame;
}

std::ostream& operator << (std::ostream& out, const Frame& f) {
	out << "(1 " << opencv::type2str((*f).type()) << " frame, " << (*f).cols << "x" << (*f).rows << ")";
	return out;
}

} }
