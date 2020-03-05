#pragma once

#include <vector>

#include "index.h"

namespace lightfields {

class BFSVisitors {
	public:
		BFSVisitors(const V2i& size, std::size_t layer_count);

		bool visited(const Index& index) const;
		Index parent(const Index& index) const;

		void visit(const Index& index, const Index& parent);

		void clear();

	private:
		std::size_t vec2index(const Index& v) const;
		Index index2vec(std::size_t i) const;

		V2i m_size;
		std::size_t m_layerCount, m_layerSize;

		std::size_t m_stage, m_mask, m_shift;
		std::vector<std::size_t> m_values;
};

}
