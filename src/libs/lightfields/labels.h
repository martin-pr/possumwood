#pragma once

#include <vector>

namespace lightfields {

// find the gap optimisation
// based on Cherkassky, Boris V. "A fast algorithm for computing maximum flow in a network." Collected Papers 3 (1994): 90-96.
class Labels {
	public:
		Labels(std::size_t size);

		std::size_t size() const;

		unsigned& operator[](std::size_t index);
		const unsigned& operator[](std::size_t index) const;

		typedef std::vector<unsigned>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		typedef std::vector<unsigned>::iterator iterator;
		iterator begin();
		iterator end();

	protected:
	private:
		std::vector<unsigned> m_labels;
};

}
