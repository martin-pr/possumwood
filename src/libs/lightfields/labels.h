#pragma once

#include <vector>

namespace lightfields {

// find the gap optimisation
// based on Cherkassky, Boris V. "A fast algorithm for computing maximum flow in a network." Collected Papers 3 (1994): 90-96.
class Labels {
	public:
		Labels(std::size_t size, unsigned label_limit);

		void clear(unsigned default_val);

		std::size_t size() const;

		class Proxy {
			public:
				operator unsigned () const;
				Proxy& operator = (unsigned val);

			private:
				Proxy(Labels* parent, std::size_t index);

				Labels* m_parent;
				std::size_t m_index;

			friend class Labels;
		};

		Proxy operator[](std::size_t index);
		const unsigned& operator[](std::size_t index) const;

		struct Gap {
			unsigned min, max;
		};

		std::vector<Gap> gaps() const;

		/// find the gap optimisation
		/// based on Cherkassky, Boris V. "A fast algorithm for computing maximum flow in a network." Collected Papers 3 (1994): 90-96.
		void relabelGap();

	protected:
	private:
		std::vector<unsigned> m_labels, m_counters;
		unsigned m_limit, m_maxLabel;

	friend class Proxy;
};

}
