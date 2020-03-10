#pragma once

#include <vector>

#include "vec2.h"

namespace lightfields {

class TLinks {
	public:
		class Edge {
			public:
				// represents an edge with two directions of flow
				Edge(int capacity = 0);

				void setCapacity(const int& capacity);
				const int& capacity() const;

				void addFlow(const int& flow);
				const int& flow() const;

				int residualCapacity() const;

			private:
				int m_capacity, m_flow;
		};

		TLinks(const V2i& size);

		Edge& edge(const V2i& i);
		const Edge& edge(const V2i& i) const;

		const V2i& size() const;

	private:
		V2i m_size;
		std::vector<Edge> m_data;
};

}
