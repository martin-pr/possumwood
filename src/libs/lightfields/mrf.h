#pragma once

#include <vector>

#include "vec2.h"

namespace lightfields {

/// A simple implementation of Markov Random Field on a regular grid, with integer values
class MRF {
	public:
		struct Value {
			Value() : value(0), confidence(0.0f) {
			}

			int value;
			float confidence;
		};

		MRF(const V2i& size);

		Value& operator[](const V2i& index);
		const Value& operator[](const V2i& index) const;

	private:
		V2i m_size;
		std::vector<Value> m_nodes;
};

}
