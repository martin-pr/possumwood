#pragma once

namespace lightfields {

class Link {
	public:
		class Direction {
			public:
				int capacity() const;

				void addFlow(const int& f);
				int flow() const;

				int residualCapacity() const;

			private:
				Direction(Link* parent, bool forward);

				Direction(const Direction&) = delete;
				Direction& operator = (const Direction&) = delete;

				Link* m_parent;
				bool m_forward;

				friend class Link;
		};

		// represents an Link with two directions of flow
		Link(int capacity);

		Link(const Link& e);
		Link& operator = (const Link&) = delete;

		Direction& forward();
		const Direction& forward() const;

		Direction& backward();
		const Direction& backward() const;

	private:
		friend class Direction;

		Direction m_forward, m_backward;

		int m_capacity, m_flow;
};


}
