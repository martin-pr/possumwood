#pragma once

#include <vector>
#include <map>

#include "index.h"

// #define BFS_VISITORS_TRIVIAL

namespace lightfields {

class BFSVisitors {
	public:
		BFSVisitors(const V2i& size, std::size_t layer_count);

		bool visited(const Index& index) const;
		Index parent(const Index& index) const;

		void visit(const Index& index, const Index& parent);

	private:
		#ifndef BFS_VISITORS_TRIVIAL
		std::size_t vec2index(const Index& v) const;
		Index index2vec(std::size_t i) const;

		Vec2<std::size_t> m_size;
		std::size_t m_layerCount, m_layerSize;

		std::vector<std::size_t> m_values;

		#else
		std::map<Index, Index> m_visited;
		#endif
};

}
