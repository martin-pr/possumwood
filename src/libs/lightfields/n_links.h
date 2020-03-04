#pragma once

#include <vector>

#include "vec2.h"

namespace lightfields {

/// A simple representation of an implicit 2D graph with 4-neighbourhood
class NLinks {
	private:
		class Edge;

	public:
		NLinks(const V2i& size, int n_link_value);

		class Direction {
			public:
				int capacity() const;

				void addFlow(const int& f);
				int flow() const;

				int residualCapacity() const;

			private:
				Direction(Edge* parent, bool forward);

				Direction(const Direction&) = delete;
				Direction& operator = (const Direction&) = delete;

				Edge* m_parent;
				bool m_forward;

				friend class Edge;
		};

		Direction& edge(const V2i& src, const V2i& dest);
		const Direction& edge(const V2i& src, const V2i& dest) const;

	private:
		std::size_t h_index(const V2i& i) const;
		std::size_t v_index(const V2i& i) const;

		class Edge {
			public:
				// represents an edge with two directions of flow
				Edge(int capacity);

				Edge(const Edge& e);
				Edge& operator = (const Edge&) = delete;

				Direction& forward();
				const Direction& forward() const;

				Direction& backward();
				const Direction& backward() const;

			private:
				friend class Direction;

				Direction m_forward, m_backward;

				int m_capacity, m_flow;
		};

		V2i m_size;
		std::vector<Edge> m_horiz, m_vert;
};

}
