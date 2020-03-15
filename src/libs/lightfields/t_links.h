#pragma once

#include <vector>

#include "vec2.h"
#include "link.h"

namespace lightfields {

class TLinks {
	public:
		TLinks(const V2i& size);

		Link& edge(const V2i& i);
		const Link& edge(const V2i& i) const;

		const V2i& size() const;

	private:
		V2i m_size;
		std::vector<Link> m_data;
};

}
