#include "pmf.h"

#include <cmath>
#include <cassert>

namespace lightfields {

PMF PMF::fromConfidence(float c, unsigned value, unsigned size) {
	assert(value < size && size > 0);
	assert(c >= 0.0f && c <= 1.0f);

	PMF result(size);
	result.m_p = std::vector<float>(size, (1.0f-c) / (float)size);
	result.m_p[value] += c;

	return result;
}

PMF::PMF(unsigned n) {
	assert(n > 0);
	m_p = std::vector<float>(n, 1.0f / (float)n);
}

std::size_t PMF::size() const {
	return m_p.size();
}

float PMF::p(unsigned index) const {
	assert(index < size());

	return m_p[index]; // ignoring weights to maintain valid PMF - weights are only relevant for sums of PMFs
}

unsigned PMF::max() const {
	assert(!m_p.empty());

	unsigned index = 0;
	float val = m_p[0];

	for(std::size_t a=1;a<m_p.size();++a)
		if(val < m_p[a]) {
			val = m_p[a];
			index = a;
		}

	return index;
}

PMF PMF::combine(const PMF& p1, float w1, const PMF& p2, float w2) {
	// this should be implemented as a convolution. However, we'd need to normalize the result afterwards anyway, which will lead to simple sum no matter what.
	assert(p1.size() == p2.size());
	assert(w1 >= 0.0f && w2 >= 0.0f);

	const float norm = w1 + w2;

	PMF result(p1.size());
	for(unsigned a=0;a<result.size();++a)
		result.m_p[a] = (p1.m_p[a]*w1 + p2.m_p[a]*w2) / norm;

	return result;
}

//////////////////////

JointPMF JointPMF::difference(unsigned size, float widthMutiplier) {
	assert(size > 0);
	assert(widthMutiplier > 0.0f);

	// compute the normalization coefficient to make sure this is a valid PMF

	// diagonal element
	float norm = (float)size; // * e^0
	// off-diagonal elements of a symmetric matrix e^(-|x-y|/w)
	for(unsigned a=1;a<size;++a)
		norm += 2.0f * float(size-a) * std::exp(-std::abs(float(a)) / widthMutiplier);

	// procedural representation of the Joint PMF
	JointPMF result(size);
	result.m_fn = [norm, widthMutiplier](unsigned row, unsigned col) {
		if(row > col)
			return std::exp(-std::abs(float(row-col)) / widthMutiplier) / norm;
		return std::exp(-std::abs(float(col-row)) / widthMutiplier) / norm;
	};

	return result;
}

JointPMF::JointPMF(unsigned size) : JointPMF(size, size) {
}

JointPMF::JointPMF(unsigned rows, unsigned cols) : m_rows(rows), m_cols(cols) {
	assert(rows > 0 && cols > 0);

	const float invsize = 1.0f / (float)(rows*cols);
	m_fn = [invsize](unsigned row, unsigned col) {
		return invsize;
	};
}

unsigned JointPMF::rows() const {
	return m_rows;
}

unsigned JointPMF::cols() const {
	return m_cols;
}

float JointPMF::p(unsigned row, unsigned col) const {
	return m_fn(row, col);
}

//////////////////////

PMF operator +(const PMF& p1, const PMF& p2) {
	// this should be implemented as a convolution. However, we'd need to normalize the result afterwards anyway, which will lead to simple sum no matter what.
	assert(p1.size() == p2.size());

	PMF result(p1.size());
	for(unsigned a=0;a<p1.size();++a)
		result.m_p[a] = (p1.m_p[a] + p2.m_p[a]) / 2.0f;

	return result;
}

PMF operator *(const PMF& p1, const PMF& p2) {
	assert(p1.size() == p2.size());

	PMF result(p1.size());
	float norm = 0.0f;
	for(unsigned a=0;a<p1.size();++a) {
		result.m_p[a] = p1.m_p[a] * p2.m_p[a];
		norm += result.m_p[a];
	}

	for(unsigned a=0;a<p1.size();++a)
		result.m_p[a] /= norm;

	return result;
}

PMF operator *(const PMF& p, const JointPMF& j) {
	assert(p.size() == j.rows());

	PMF result(j.cols());

	for(unsigned col=0; col<j.cols(); ++col) {
		result.m_p[col] = 0.0f;
		for(unsigned row=0; row<j.rows(); ++row)
			result.m_p[col] += p.p(row) * j.p(row, col);
	}

	return result;
}

PMF operator *(const JointPMF& j, const PMF& p) {
	assert(p.size() == j.cols());

	PMF result(j.rows());

	for(unsigned row=0; row<j.rows(); ++row) {
		result.m_p[row] = 0.0f;
		for(unsigned col=0; col<j.cols(); ++col)
			result.m_p[row] += j.p(row, col) * p.p(col);
	}

	return result;
}

}
