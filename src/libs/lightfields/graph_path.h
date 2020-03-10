#pragma once

#include <vector>

#include "index.h"

namespace lightfields {

class GraphPath {
	public:
		GraphPath();

		bool isValid() const;
		bool empty() const;

		/// designed to be filled from the back - this adds a link to the "front" of the path
		void add(const Index& link);
		void clear();

		typedef std::vector<Index>::const_reverse_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		const Index& front() const;
		const Index& back() const;

	private:
		std::vector<Index> n_links;
};

}
