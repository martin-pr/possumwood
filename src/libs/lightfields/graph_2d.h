#pragma once

#include <ImathVec.h>

#include <vector>

namespace lightfields {

/// A simple representation of an implicit 2D graph with 4-neighbourhood
class Graph2D {
	private:
		class Edge;

	public:
		Graph2D(const Imath::V2i& size, float n_link_value);

		static constexpr float flowEPS() { return 1e-5; };

		class Direction {
			public:
				float capacity() const;

				void addFlow(const float& f);
				float flow() const;

				float residualCapacity() const;

			private:
				Direction(Edge* parent, bool forward);

				Direction(const Direction&) = delete;
				Direction& operator = (const Direction&) = delete;

				Edge* m_parent;
				bool m_forward;

				friend class Edge;
		};

		Direction& edge(const Imath::V2i& src, const Imath::V2i& dest);
		const Direction& edge(const Imath::V2i& src, const Imath::V2i& dest) const;

	private:
		std::size_t h_index(const Imath::V2i& i) const;
		std::size_t v_index(const Imath::V2i& i) const;

		class Edge {
			public:
				// represents an edge with two directions of flow
				Edge(float capacity);

				Edge(const Edge& e);
				Edge& operator = (const Edge&) = delete;

				Direction& forward();
				const Direction& forward() const;

				Direction& backward();
				const Direction& backward() const;

			private:
				friend class Direction;

				Direction m_forward, m_backward;

				float m_capacity, m_flow;
		};

		Imath::V2i m_size;
		std::vector<Edge> m_horiz, m_vert;
};

}
