#include "lenslet_graph.h"

namespace lightfields {

namespace {

struct Vec2Compare {
	bool operator() (const cv::Vec2f& v1, const cv::Vec2f& v2) const {
		if(v1[0] != v2[0])
			return v1[0] < v2[0];
		return v1[1] < v2[1];
	}
};

struct Lenslet {
	Lenslet(const cv::Vec2f& c) : center(c), neighbourCount(0) {
	}

	void addNeighbour(std::size_t index) {
		assert(neighbourCount < 6);

		neighbours[neighbourCount] = index;
		++neighbourCount;
	}

	cv::Vec2f center;
	std::array<std::size_t, 6> neighbours;
	unsigned char neighbourCount;
};

}

/////////////

struct LensletGraph::Pimpl {
	cv::Vec2i sensorSize;
	int exclusionBorder;

	std::vector<Lenslet> lenslets;
};

/////////////

LensletGraph::LensletGraph(cv::Vec2i sensorSize, int exclusionBorder) : m_pimpl(new Pimpl()) {
	m_pimpl->sensorSize = sensorSize;
	m_pimpl->exclusionBorder = exclusionBorder;
}

LensletGraph::~LensletGraph() {
}

void LensletGraph::addLenslet(const cv::Vec2f& center) {
	m_pimpl->lenslets.push_back(Lenslet(center));
}

cv::Matx<double, 3, 3> LensletGraph::fit() {
	// build the subdiv and index
	std::map<cv::Vec2f, std::size_t, Vec2Compare> index;

	cv::Subdiv2D subdiv(cv::Rect(0, 0, m_pimpl->sensorSize[0], m_pimpl->sensorSize[1]));
	for(auto& l : m_pimpl->lenslets) {
		subdiv.insert(cv::Point2f(l.center[0], l.center[1]));
		index.insert(std::make_pair(cv::Point2f(l.center[0], l.center[1]), index.size()));
	}

	// collect and process the edges
	std::vector<cv::Vec4f> srcEdgeList;
	subdiv.getEdgeList(srcEdgeList);

	for(auto eit = srcEdgeList.begin(); eit != srcEdgeList.end(); ++eit) {
		auto& e = *eit;

		if(e[0] > m_pimpl->exclusionBorder && e[0] < m_pimpl->sensorSize[0] - m_pimpl->exclusionBorder && e[1] > m_pimpl->exclusionBorder && e[1] < m_pimpl->sensorSize[1] - m_pimpl->exclusionBorder &&
			e[2] > m_pimpl->exclusionBorder && e[2] < m_pimpl->sensorSize[0] - m_pimpl->exclusionBorder && e[3] > m_pimpl->exclusionBorder && e[3] < m_pimpl->sensorSize[1] - m_pimpl->exclusionBorder) {

			auto it1 = index.find(cv::Point2f(e[0], e[1]));
			auto it2 = index.find(cv::Point2f(e[2], e[3]));
			assert(it1 != index.end() && it2 != index.end());

			m_pimpl->lenslets[it1->second].addNeighbour(it2->second);
			m_pimpl->lenslets[it2->second].addNeighbour(it1->second);
		}
	}
	return cv::Matx<double, 3, 3>();
}

void LensletGraph::drawCenters(cv::Mat& target) const {
	assert(target.rows == m_pimpl->sensorSize[0] && target.cols == m_pimpl->sensorSize[1]);
	assert(target.type() == CV_8UC1);

	for(auto& l : m_pimpl->lenslets)
		target.at<unsigned char>(l.center[1], l.center[0]) = 255;
}

void LensletGraph::drawEdges(cv::Mat& target) const {
	assert(target.rows == m_pimpl->sensorSize[0] && target.cols == m_pimpl->sensorSize[1]);
	assert(target.type() == CV_8UC1);

	for(auto& l1 : m_pimpl->lenslets) {
		for(unsigned char n=0;n<l1.neighbourCount;++n) {
			auto& l2 = m_pimpl->lenslets[l1.neighbours[n]];

			const float a = atan((l2.center[1] - l1.center[1]) / (l2.center[0] - l1.center[0])) / M_PI * 2.0;

			float color = 155.0;
			if(a < -0.33)
				color = 55.0;
			if(a > 0.33)
				color = 255;

			cv::line(target, cv::Point2i(l1.center[0], l1.center[1]), cv::Point2i(l2.center[0], l2.center[1]), cv::Scalar(color));
		}
	}
}

}
