#include "circles_grid.h"

#include "tools.h"

namespace possumwood { namespace opencv {

CirclesGrid::CirclesGrid(const cv::Mat& data, const cv::Size& size, bool wasFound, const Type& type) : m_size(size), m_wasFound(wasFound), m_type(type) {
	m_features = std::shared_ptr<const cv::Mat>(new cv::Mat(data.clone()));
}

const cv::Mat& CirclesGrid::operator*() const {
	return *m_features;
}

const cv::Size& CirclesGrid::size() const {
	return m_size;
}

bool CirclesGrid::wasFound() const {
	return m_wasFound;
}

CirclesGrid::Type CirclesGrid::type() const {
	return m_type;
}

bool CirclesGrid::operator == (const CirclesGrid& f) const {
	return m_features == f.m_features;
}

bool CirclesGrid::operator != (const CirclesGrid& f) const {
	return m_features != f.m_features;
}

std::ostream& operator << (std::ostream& out, const CirclesGrid& f) {
	out << "(" << (*f).rows << " circles, size = " << f.size().width << "x" << f.size().height << ", found=" << f.wasFound() << ")";
	return out;
}

} }
