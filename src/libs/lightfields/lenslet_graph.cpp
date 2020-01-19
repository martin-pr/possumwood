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
	Lenslet(const cv::Vec2f& c) : center(c), neighbourCount(0), id(INT_MIN, INT_MIN) {
	}

	void addNeighbour(std::size_t index) {
		assert(neighbourCount < 6);

		neighbours[neighbourCount] = index;
		++neighbourCount;
	}

	cv::Vec2f center;
	std::array<std::size_t, 6> neighbours;
	unsigned char neighbourCount;
	cv::Vec2i id;
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

namespace {

/// computes relative integer offset between two lenslets (assumed to be neighbouring edges of a graph)
cv::Vec2i offset(const cv::Vec2f& l1, const cv::Vec2f& l2) {
	const float angle = atan2(l2[1] - l1[1], l2[0] - l1[0]) / M_PI * 6.0f;
	assert(angle >= -6.0f && angle <= 6.0f);

	if(angle < -5.0f || angle > 5.0f)
		return cv::Vec2i(-1, 0);

	if(angle < -3.0f)
		return cv::Vec2i(0, -1);

	if(angle > 3.0f)
		return cv::Vec2i(-1, 1);

	if(angle < -1.0f)
		return cv::Vec2i(1, -1);

	if(angle > 1.0f)
		return cv::Vec2i(0, 1);

	return cv::Vec2i(1, 0);
}

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
	{
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
	}

	// propagate IDs
	{
		// find first connected lenslet
		auto it = m_pimpl->lenslets.begin();
		while(it != m_pimpl->lenslets.end() && it->neighbourCount == 0)
			++it;

		if(it != m_pimpl->lenslets.end()) {
			// this lenslet is the "origin" now
			it->id = cv::Vec2i(0, 0);

			// initialise the edges (to-be-processed) list
			std::vector<std::pair<std::size_t, std::size_t>> edges;
			for(std::size_t n=0; n<it->neighbourCount; ++n)
				edges.push_back(std::make_pair(it - m_pimpl->lenslets.begin(), it->neighbours[n]));

			while(!edges.empty()) {
				// get the processed edge
				auto edge = edges.back();
				edges.pop_back();

				const Lenslet& source = m_pimpl->lenslets[edge.first];
				Lenslet& target = m_pimpl->lenslets[edge.second];

				// assert the "origin" lenslet, which should have been processed already
				assert(source.id[0] > INT_MIN && source.id[1] > INT_MIN);

				// get the "offset" based on edge direction
				const cv::Vec2i off = offset(source.center, target.center);

				// either set the target, or assert its value is right
				if(target.id[0] > INT_MIN) {
					assert(target.id[0] == source.id[0] + off[0]);
					assert(target.id[1] == source.id[1] + off[1]);
				}

				else {
					target.id[0] = source.id[0] + off[0];
					target.id[1] = source.id[1] + off[1];

					// recursively continue around all edges of the target
					for(std::size_t n=0; n<target.neighbourCount; ++n)
						edges.push_back(std::make_pair(edge.second, target.neighbours[n]));
				}
			}
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

void LensletGraph::drawFit(cv::Mat& target) const {
	assert(target.rows == m_pimpl->sensorSize[0] && target.cols == m_pimpl->sensorSize[1]);
	assert(target.type() == CV_8UC1);

	for(auto& l : m_pimpl->lenslets)
		cv::circle(target, cv::Point(l.center[0], l.center[1]), 2, (l.id[0] + 2560) % 256, 2);
}

}
