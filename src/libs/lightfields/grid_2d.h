#pragma once

#include <ImathVec.h>

#include <vector>

namespace lightfields {

class Grid2D {
	public:
		struct Edge {
			// represents an edge with two directions of flow
			Edge(float val = 0.0f) : forward(val), backward(val) {
			}

			float forward, backward;
		};

		Grid2D(const Imath::V2i& size);

		Edge& edge(const Imath::V2i& i);
		const Edge& edge(const Imath::V2i& i) const;

		const Imath::V2i& size() const;

	private:
		Imath::V2i m_size;
		std::vector<Edge> m_data;
};

}
