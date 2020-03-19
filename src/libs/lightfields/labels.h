#pragma once

#include <vector>

namespace lightfields {

// find the gap optimisation
// based on Cherkassky, Boris V. "A fast algorithm for computing maximum flow in a network." Collected Papers 3 (1994): 90-96.
class Labels {
	public:
		Labels(std::size_t size, unsigned max_label);

		std::size_t size() const;

		unsigned& operator[](std::size_t index);
		const unsigned& operator[](std::size_t index) const;

		/// find the gap optimisation
		/// based on Cherkassky, Boris V. "A fast algorithm for computing maximum flow in a network." Collected Papers 3 (1994): 90-96.
		void relabelGap();

	protected:
	private:
		std::vector<unsigned> m_labels;
		unsigned m_maxLabel;
};

}
