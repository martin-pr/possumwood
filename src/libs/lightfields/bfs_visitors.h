#pragma once

#include <vector>

#include "vec2.h"

namespace lightfields {

class BFSVisitors {
	public:
		BFSVisitors(const V2i& size);

		bool visited(const V2i& index) const;
		V2i parent(const V2i& index) const;

		void visit(const V2i& index, const V2i& parent);

		void clear();

	private:
		std::size_t vec2index(const V2i& v) const;
		V2i index2vec(std::size_t i) const;

		V2i m_size;
		std::size_t m_stage, m_mask, m_shift;
		std::vector<std::size_t> m_values;
};

}
