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

template <unsigned DEGREE, typename FN>
struct Visitor {
	std::array<float, DEGREE * 4> b_coeffs;
	std::array<std::size_t, DEGREE> offsets;
	std::array<std::size_t, DEGREE> subdiv;
	FN fn;
};

template <typename VISITOR, unsigned DIM>
struct Visit {
	static void visit(const VISITOR& visitor, std::size_t index = 0, float weight = 1.0f) {
		index = index * (visitor.subdiv[DIM] + 2) + visitor.offsets[DIM];
		for(std::size_t a = 0; a < 4; ++a)
			Visit<VISITOR, DIM - 1>::visit(visitor, index + a, visitor.b_coeffs[DIM * 4 + a] * weight);
	}
};

template <typename VISITOR>
struct Visit<VISITOR, 0> {
	static void visit(const VISITOR& visitor, std::size_t index, float weight) {
		index = index * (visitor.subdiv[0] + 2) + visitor.offsets[0];
		for(std::size_t a = 0; a < 4; ++a)
			visitor.fn(index + a, weight * visitor.b_coeffs[a]);
	}
};

}  // namespace

template <unsigned DEGREE>
template <typename FN>
void BSpline<DEGREE>::visit(const std::array<float, DEGREE>& _coords, const FN& fn) const {
	Visitor<DEGREE, FN> data;

	std::array<float, DEGREE> coords = _coords;
	data.subdiv = m_subdiv;
	data.fn = fn;

	for(unsigned d = 0; d < DEGREE; ++d) {
		coords[d] = (coords[d] - m_min[d]) / (m_max[d] - m_min[d] + 1e-6f);

		if(coords[d] < 0.0 || coords[d] >= 1.0) {
			std::stringstream ss;
			ss << "Coordinate #" << d << " out of range - " << _coords[d] << " (expected between " << m_min[d]
			   << " and " << m_max[d] << ")";
			throw std::runtime_error(ss.str());
		}

		coords[d] *= (float)(m_subdiv[d]);

		data.offsets[d] = floor(coords[d]);
		coords[d] = coords[d] - data.offsets[d];

		assert(coords[d] >= 0.0f);
		assert(coords[d] < 1.0f);

		assert(data.offsets[d] >= std::size_t(0) && data.offsets[d] < m_subdiv[d]);
	}

	// precompute per-dim b-spline coeficients
	for(unsigned d = 0; d < DEGREE; ++d)
		for(unsigned a = 0; a < 4; ++a)
			data.b_coeffs[d * 4 + a] = B(coords[d], a);

	// evaluate the visitor
	Visit<Visitor<DEGREE, FN>, DEGREE - 1>::visit(data);
}

namespace {

struct AddSample {
	std::vector<std::pair<float, float>>* m_controls;
	float m_value;

	void operator()(std::size_t index, float weight) const {
		(*m_controls)[index].first += weight * m_value;
		(*m_controls)[index].second += weight;
	}
};

struct Sample {
	const std::vector<std::pair<float, float>>* m_controls;
	float* m_result;

	void operator()(std::size_t index, float weight) const {
		if((*m_controls)[index].second > 0.0f)
			(*m_result) += (*m_controls)[index].first / (*m_controls)[index].second * weight;
	}
};

}  // namespace

template <unsigned DEGREE>
void BSpline<DEGREE>::addSample(const std::array<float, DEGREE>& coords, float value) {
	AddSample add{&m_controls, value};
	visit(coords, add);
}

template <unsigned DEGREE>
float BSpline<DEGREE>::sample(const std::array<float, DEGREE>& coords) const {
	float result = 0;
	Sample smp{&m_controls, &result};
	this->visit(coords, smp);

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
