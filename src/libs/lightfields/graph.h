#pragma once

#include <vector>
#include <set>

#include <opencv2/opencv.hpp>

#include <tbb/blocked_range2d.h>

#include "n_links.h"
#include "t_links.h"
#include "index.h"
#include "active_queue.h"

namespace lightfields {

class BFSVisitors;

class Graph {
	public:
		Graph(const V2i& size, int n_link_value, std::size_t layer_count);

		void setValue(const V2i& pos, const std::vector<int>& values);

		void edmondsKarpSolve();
		void pushRelabelSolve();

		cv::Mat minCut() const;
		std::vector<cv::Mat> debug() const;

	private:
		std::size_t v2i(const V2i& v) const;
		Index val2index(std::size_t value) const;
		std::size_t index2val(const Index& i) const;

		bool iterate(const tbb::blocked_range2d<int>& range);

		int flow(const BFSVisitors& visitors, const Index& end) const;
		void doFlow(const BFSVisitors& visitors, const Index& end, int value);

		bool pushHoriz(const Index& current, const Index& next, const std::vector<int>& label, int& excess, ActiveQueue& queue);
		bool pushVertDown(const Index& current, const std::vector<int>& label, int& excess, ActiveQueue& queue);
		bool pushVertUp(const Index& current, const std::vector<int>& label, int& excess, ActiveQueue& queue);
		bool relabel(const Index& current, std::vector<int>& label) const;

		cv::Mat t_flow(const TLinks& t) const;
		cv::Mat n_flow(const NLinks& t) const;

		V2i m_size;
		std::vector<TLinks> m_tLinks;
		std::vector<NLinks> m_nLinks;
};

}
