#pragma once

#include <vector>
#include <set>

#include <ImathVec.h>

#include <opencv2/opencv.hpp>

#include "graph_2d.h"
#include "grid_2d.h"

namespace lightfields {

class Graph {
	public:
		struct Edge {
			// represents an edge with two directions of flow
			Edge(int val = 0.0f) : forward(val), backward(val) {
			}

			int forward, backward;
		};

		Graph(const Imath::V2i& size, int n_link_value);

		void setValue(const Imath::V2i& pos, int source_weight, int sink_weight);

		struct SetComparator {
			bool operator()(const Imath::V2i& v1, const Imath::V2i& v2) const {
				if(v1[1] != v2[1])
					return v1[1] < v2[1];
				return v1[0] < v2[0];
			}
		};

		void solve();

		cv::Mat minCut() const;

		cv::Mat sourceFlow() const;
		cv::Mat sinkFlow() const;
		cv::Mat horizontalFlow() const;
		cv::Mat verticalFlow() const;

	private:
		std::size_t v2i(const Imath::V2i& v) const;
		Imath::V2i i2v(std::size_t v) const;

		struct Path {
			Path();

			bool isValid() const;

			std::vector<Imath::V2i> n_links;
		};

		/// Simplified depth-first-search - only forward search from S and backward search from T (will avoid loops implicitly).
		/// Indices - first index into source links, last index into sink links, mid indices horiz if id < horiz_n_links.size(), vert otherwise
		bool bfs_2(Path& path, std::size_t& offset) const;

		int flow(const Path& path) const;

		// void collect(std::set<Imath::V2i, Graph::SetComparator>& subgraph, const Imath::V2i& i) const;

		Imath::V2i m_size;
		Grid2D m_sourceLinks, m_sinkLinks;
		Graph2D m_nLinks;
};

}
