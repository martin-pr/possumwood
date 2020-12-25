#include <algorithm>

#include <opencv2/opencv.hpp>

#include "bspline.h"

namespace possumwood {
namespace opencv {

template <unsigned DEGREE>
float BSpline<DEGREE>::B(float t, unsigned k) {
	assert(t >= 0.0 && t <= 1.0);
	assert(k < 4);

	switch(k) {
		case 0:
			t = 1.0 - t;
			return t * t * t / 6.0;
		case 1:
			return (3.0 * t * t * t - 6.0 * t * t + 4.0) / 6.0;
		case 2:
			return (-3.0 * t * t * t + 3.0 * t * t + 3.0 * t + 1.0) / 6.0;
		case 3:
			return t * t * t / 6.0;
		default:
			return 0.0;
	}
}

template <unsigned DEGREE>
template <typename T>
std::array<T, DEGREE> BSpline<DEGREE>::initArray(T val) {
	std::array<T, DEGREE> result;
	result.fill(val);
	return result;
}

template <unsigned DEGREE>
BSpline<DEGREE>::BSpline(const std::array<std::size_t, DEGREE>& subdiv,
                         const std::array<float, DEGREE>& min,
                         const std::array<float, DEGREE>& max)
    : m_subdiv(subdiv), m_min(min), m_max(max) {
	std::size_t controlCount = 1;
	for(unsigned d = 0; d < DEGREE; ++d)
		controlCount *= subdiv[d] + 2;
	m_controls.reserve(controlCount);
	m_controls.resize(controlCount, std::make_pair(0.0f, 0.0f));
}

namespace {

struct Index {
	std::size_t index;
	float weight;
};

template <unsigned DEGREE, unsigned DIM = DEGREE - 1>
struct IndexMaker {
	static Index makeIndex(const std::array<float, DEGREE * 4>& b_coeffs,
	                       const std::array<int, DEGREE>& offsets,
	                       const std::array<std::size_t, DEGREE>& subdiv,
	                       std::size_t base) {
		Index result{base % 4 + offsets[DIM], b_coeffs[DIM * 4 + base % 4]};
		const Index& tmp = IndexMaker<DEGREE, DIM - 1>::makeIndex(b_coeffs, offsets, subdiv, base / 4);
		result.index += (subdiv[DIM] + 2) * tmp.index;
		result.weight *= tmp.weight;

		return result;
	}
};

template <unsigned DEGREE>
struct IndexMaker<DEGREE, 0> {
	static Index makeIndex(const std::array<float, DEGREE * 4>& b_coeffs,
	                       const std::array<int, DEGREE>& offsets,
	                       const std::array<std::size_t, DEGREE>& subdiv,
	                       std::size_t base) {
		return Index{base % 4 + offsets[0], b_coeffs[base % 4]};
	}
};

}  // namespace

template <unsigned DEGREE>
template <typename FN>
void BSpline<DEGREE>::visit(const std::array<float, DEGREE>& _coords, const FN& fn) const {
	std::array<float, DEGREE> coords = _coords;
	std::array<int, DEGREE> offset;

	for(unsigned d = 0; d < DEGREE; ++d) {
		coords[d] = (coords[d] - m_min[d]) / (m_max[d] - m_min[d] + 1e-6f);

		if(coords[d] < 0.0 || coords[d] >= 1.0) {
			std::stringstream ss;
			ss << "Coordinate #" << d << " out of range - " << _coords[d] << " (expected between " << m_min[d]
			   << " and " << m_max[d] << ")";
			throw std::runtime_error(ss.str());
		}

		coords[d] *= (float)(m_subdiv[d]);

		offset[d] = floor(coords[d]);
		coords[d] = coords[d] - offset[d];

		assert(coords[d] >= 0.0f);
		assert(coords[d] < 1.0f);

		assert(offset >= 0 && offset < m_subdiv[d]);
	}

	// precompute per-dim b-spline coeficients
	std::array<float, DEGREE * 4> b_coeffs;
	for(unsigned d = 0; d < DEGREE; ++d)
		for(unsigned a = 0; a < 4; ++a)
			b_coeffs[d * 4 + a] = B(coords[d], a);

	// fill the coeff values
	const int end = pow(4, DEGREE);
	for(int i = 0; i < end; ++i) {
		const Index& index = IndexMaker<DEGREE>::makeIndex(b_coeffs, offset, m_subdiv, i);
		fn(index.index, index.weight);
	}
}

template <unsigned DEGREE>
void BSpline<DEGREE>::addSample(const std::array<float, DEGREE>& coords, float value) {
	visit(coords, [&](unsigned index, float weight) {
		m_controls[index].first += weight * value;
		m_controls[index].second += weight;
	});
}

template <unsigned DEGREE>
float BSpline<DEGREE>::sample(const std::array<float, DEGREE>& coords) const {
	float result = 0;
	visit(coords, [&](unsigned index, float weight) {
		if(m_controls[index].second > 0.0)
			result += m_controls[index].first / m_controls[index].second * weight;
	});

	assert(std::isfinite(result));

	return result;
}

template <unsigned DEGREE>
bool BSpline<DEGREE>::operator==(const BSpline& b) const {
	return m_subdiv == b.m_subdiv && m_controls == b.m_controls && m_min == b.m_min && m_max == b.m_max;
}

template <unsigned DEGREE>
bool BSpline<DEGREE>::operator!=(const BSpline& b) const {
	return m_subdiv != b.m_subdiv || m_controls != b.m_controls || m_min != b.m_min || m_max != b.m_max;
}

template <unsigned DEGREE>
std::ostream& operator<<(std::ostream& out, const BSpline<DEGREE>& spline) {
	out << "B-Spline, degree=" << DEGREE;
	return out;
}

}  // namespace opencv
}  // namespace possumwood
