#pragma once

#include <cmath>
#include <limits>

namespace lightfields {

/// A simple representation of Gaussian Probability Distribution Function.
/// Based on Bromiley, Paul. "Products and convolutions of Gaussian probability density functions." Tina-Vision Memo 3.4
/// (2003): 1.
class PDFGaussian {
  public:
	static PDFGaussian fromConfidence(float mu, float confidence) {
		assert(confidence >= 0.0f && confidence <= 1.0f);

		if(confidence > 1e-5f)
			// return PDFGaussian(mu, -std::log(confidence));
			return PDFGaussian(mu, 1.0f / confidence - 1.0f);
		return PDFGaussian(mu, std::numeric_limits<float>::infinity());
	}

	PDFGaussian(float mu, float sigma) : m_mu(mu), m_sigma(sigma) {
	}

	float p(float val) const {
		if(std::isinf(m_sigma))
			return 0.0f;

		return 1.0f / (m_sigma * sqrt(2.0f * M_PI)) * std::exp(-0.5f * std::pow((val - m_mu) / m_sigma, 2));
	}

	float sigma() const {
		return m_sigma;
	}

	float mu() const {
		return m_mu;
	}

	float confidence() const {
		return std::exp(-m_sigma);
	}

	static PDFGaussian pow(const PDFGaussian& p, float power) {
		assert(power >= 0.0f && "Only positive powers supported (for now)");

		if(power == 0.0f)
			return fromConfidence(0, 0);

		return PDFGaussian(p.mu(), p.sigma() / std::sqrt(power));
	}

  private:
	float m_mu, m_sigma;
};

PDFGaussian operator+(const PDFGaussian& p1, const PDFGaussian& p2) {
	// implements the product of two gaussian distributions
	// (i.e., does NOT implement the sum of the resulting value!)

	// using limits
	if(std::isinf(p2.sigma()))
		return p1;
	if(std::isinf(p1.sigma()))
		return p2;

	// using the actual equation
	return PDFGaussian((p2.mu() * std::pow(p1.sigma(), 2) + p1.mu() * std::pow(p2.sigma(), 2)) /
	                       (std::pow(p1.sigma(), 2) + std::pow(p2.sigma(), 2)),
	                   std::sqrt((std::pow(p1.sigma(), 2) * std::pow(p2.sigma(), 2)) /
	                             (std::pow(p1.sigma(), 2) + std::pow(p2.sigma(), 2))));
}

PDFGaussian operator*(const PDFGaussian& p1, const PDFGaussian& p2) {
	// implements the CONVOLUTION of two Gaussian PDFs, NOT the value multiplication
	const float mu = p1.mu() + p2.mu();

	if(std::isinf(p1.sigma()) || std::isinf(p2.sigma()))
		return PDFGaussian(mu, std::numeric_limits<float>::infinity());

	return PDFGaussian(mu, std::sqrt(std::pow(p1.sigma(), 2) + std::pow(p2.sigma(), 2)));
}

}  // namespace lightfields
