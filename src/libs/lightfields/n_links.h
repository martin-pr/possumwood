#pragma once

#include <vector>

#include "link.h"
#include "vec2.h"

namespace lightfields {

/// A simple representation of an implicit 2D graph with 4-neighbourhood
class NLinks {
  private:
	class Edge;

  public:
	NLinks(const V2i& size, int n_link_value);

	Link::Direction& edge(const V2i& src, const V2i& dest);
	const Link::Direction& edge(const V2i& src, const V2i& dest) const;

	V2i size() const;

  private:
	std::size_t h_index(const V2i& i) const;
	std::size_t v_index(const V2i& i) const;

	V2i m_size;
	std::vector<Link> m_horiz, m_vert;
};

}  // namespace lightfields
