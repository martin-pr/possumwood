#pragma once

#include <vector>
#include <set>

#include <opencv2/opencv.hpp>

#include "n_links.h"
#include "t_links.h"
#include "graph_path.h"

namespace lightfields {

class BFSVisitors;

class Graph {
	public:
		struct Edge {
			// represents an edge with two directions of flow
			Edge(int val = 0.0f) : forward(val), backward(val) {
			}

			int forward, backward;
		};

		Graph(const V2i& size, int n_link_value, std::size_t layer_count);

		void setValue(const V2i& pos, const std::vector<int>& values);

		void solve();

		cv::Mat minCut() const;

		std::vector<cv::Mat> debug() const;

	private:
		std::size_t v2i(const V2i& v) const;
		V2i i2v(std::size_t v) const;

		int bfs_2(GraphPath& path, std::size_t& offset) const;
		void doFlow(const GraphPath& path, int flow);

		int flow(const GraphPath& path) const;

		cv::Mat t_flow(const TLinks& t) const;
		cv::Mat n_flow(const NLinks& t) const;

		V2i m_size;
		std::vector<TLinks> m_tLinks;
		std::vector<NLinks> m_nLinks;
};

}
