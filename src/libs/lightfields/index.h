#pragma once

#include <cmath>

#include "vec2.h"

namespace lightfields {

struct Index {
	V2i pos;
	unsigned n_layer;

	bool operator == (const Index& i) const {
		return pos == i.pos && n_layer == i.n_layer;
	}

	bool operator != (const Index& i) const {
		return pos != i.pos || n_layer != i.n_layer;
	}

	bool operator < (const Index& i) const {
		if(n_layer != i.n_layer)
			return n_layer < i.n_layer;

		if(pos.y != i.pos.y)
			return pos.y < i.pos.y;

		return pos.x < i.pos.x;
	}

	static int sqdist(const Index& i1, const Index& i2) {
		if(i1.n_layer < i2.n_layer)
			return V2i::sqdist(i1.pos, i2.pos) + (i2.n_layer - i1.n_layer);
		else
			return V2i::sqdist(i1.pos, i2.pos) + (i1.n_layer - i2.n_layer);
	}
};

}
