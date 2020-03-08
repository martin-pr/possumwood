#pragma once

#include <vector>
#include <set>

#include <opencv2/opencv.hpp>

#include "n_links.h"
#include "t_links.h"
#include "index.h"

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

		void step(BFSVisitors& visitors, std::deque<Index>& q, const Index& current_v, const Index& new_v) const;

		class Path {
			public:
				Path();

				bool isValid() const;
				bool empty() const;

				/// designed to be filled from the back - this adds a link to the "front" of the path
				void add(const Index& link);
				void clear();

				typedef std::vector<Index>::const_reverse_iterator const_iterator;
				const_iterator begin() const;
				const_iterator end() const;

				const Index& front() const;
				const Index& back() const;

			private:
				std::vector<Index> n_links;
		};

		int bfs_2(Path& path, std::size_t& offset) const;

		int flow(const Path& path) const;

		cv::Mat t_flow(const TLinks& t) const;
		cv::Mat n_flow_horiz(const NLinks& t) const;
		cv::Mat n_flow_vert(const NLinks& t) const;

		V2i m_size;
		std::vector<TLinks> m_tLinks;
		std::vector<NLinks> m_nLinks;
};

}
