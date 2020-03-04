#pragma once

#include <vector>
#include <set>

#include <opencv2/opencv.hpp>

#include "n_links.h"
#include "t_links.h"

namespace lightfields {

class Graph {
	public:
		struct Edge {
			// represents an edge with two directions of flow
			Edge(int val = 0.0f) : forward(val), backward(val) {
			}

			int forward, backward;
		};

		Graph(const V2i& size, int n_link_value);

		void setValue(const V2i& pos, int source_weight, int sink_weight);

		void solve();

		cv::Mat minCut() const;

		cv::Mat sourceFlow() const;
		cv::Mat sinkFlow() const;
		cv::Mat horizontalFlow() const;
		cv::Mat verticalFlow() const;

	private:
		std::size_t v2i(const V2i& v) const;
		V2i i2v(std::size_t v) const;

		struct Path {
			Path();

			bool isValid() const;

			std::vector<V2i> n_links;
		};

		/// Simplified depth-first-search - only forward search from S and backward search from T (will avoid loops implicitly).
		/// Indices - first index into source links, last index into sink links, mid indices horiz if id < horiz_n_links.size(), vert otherwise
		int bfs_2(Path& path, std::size_t& offset) const;

		int flow(const Path& path) const;

		V2i m_size;
		TLinks m_sourceLinks, m_sinkLinks;
		NLinks m_nLinks;
};

}
