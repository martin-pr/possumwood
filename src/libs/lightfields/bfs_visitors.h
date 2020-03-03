#pragma once

#include <vector>

namespace lightfields {

class BFSVisitors {
	public:
		BFSVisitors(std::size_t count);

		bool visited(std::size_t index) const;
		std::size_t parent(std::size_t index) const;

		void visit(std::size_t index, std::size_t parent);

		void clear();

	private:
		std::size_t m_stage, m_mask, m_shift;
		std::vector<std::size_t> m_values;
};

}
