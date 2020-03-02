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
			Edge(float val = 0.0f) : forward(val), backward(val) {
			}

			float forward, backward;
		};

		static constexpr float flowEPS() { return 1e-5; };

		Graph(const Imath::V2i& size, float n_link_value);

		void setValue(const Imath::V2i& pos, float source_weight, float sink_weight);

		struct SetComparator {
			bool operator()(const Imath::V2i& v1, const Imath::V2i& v2) const {
				if(v1[1] != v2[1])
					return v1[1] < v2[1];
				return v1[0] < v2[0];
			}
		};

		void solve();

		cv::Mat minCut() const;
		// std::set<Imath::V2i, SetComparator> sinkGraph() const;

		cv::Mat sourceFlow() const;
		cv::Mat sinkFlow() const;
		cv::Mat horizontalFlow() const;
		cv::Mat verticalFlow() const;

	private:
		struct Path {
			Path();

			bool isValid() const;

			std::vector<Imath::V2i> n_links;
		};

		/// Simplified depth-first-search - only forward search from S and backward search from T (will avoid loops implicitly).
		/// Indices - first index into source links, last index into sink links, mid indices horiz if id < horiz_n_links.size(), vert otherwise
		bool bfs_2(Path& path, std::size_t& offset) const;

		float flow(const Path& path) const;

		// void collect(std::set<Imath::V2i, Graph::SetComparator>& subgraph, const Imath::V2i& i) const;

		Imath::V2i m_size;
		Grid2D m_sourceLinks, m_sinkLinks;
		Graph2D m_nLinks;
};

}
