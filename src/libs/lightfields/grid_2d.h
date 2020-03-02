#pragma once

#include <ImathVec.h>

#include <vector>

namespace lightfields {

class Grid2D {
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

		Grid2D(const Imath::V2i& size);

		Edge& edge(const Imath::V2i& i);
		const Edge& edge(const Imath::V2i& i) const;

		const Imath::V2i& size() const;

	private:
		Imath::V2i m_size;
		std::vector<Edge> m_data;
};

}
