#pragma once

#include <ImathVec.h>

#include <vector>

namespace lightfields {

/// A simple representation of an implicit 2D graph with 4-neighbourhood
class Graph2D {
	public:
		Graph2D(const Imath::V2i& size, float n_link_value);

		float& edge(const Imath::V2i& src, const Imath::V2i& dest);
		const float& edge(const Imath::V2i& src, const Imath::V2i& dest) const;

	private:
		std::size_t h_index(const Imath::V2i& i) const;
		std::size_t v_index(const Imath::V2i& i) const;

		struct Edge {
			// represents an edge with two directions of flow
			Edge(float val = 0.0f) : forward(val), backward(val) {
			}

			float forward, backward;
		};

		Imath::V2i m_size;
		std::vector<Edge> m_horiz, m_vert;
};

}
