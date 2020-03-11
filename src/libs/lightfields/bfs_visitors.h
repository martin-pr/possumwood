#pragma once

#include <vector>
#include <map>

#include "index.h"

// #define BFS_VISITORS_TRIVIAL

namespace lightfields {

class BFSVisitors {
	public:
		BFSVisitors(const V2i& size, unsigned layer_count);

		bool visited(const Index& index) const;
		Index parent(const Index& index) const;

		void visit(const Index& index, const Index& parent);

	private:
		#ifndef BFS_VISITORS_TRIVIAL
		unsigned vec2index(const Index& v) const;
		Index index2vec(unsigned i) const;

		Vec2<unsigned> m_size;
		unsigned m_layerCount, m_layerSize;

		std::vector<unsigned> m_values;

		#else
		std::map<Index, Index> m_visited;
		#endif
};

}
