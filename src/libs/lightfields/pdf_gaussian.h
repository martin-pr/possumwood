#pragma once

#include <cmath>
#include <limits>

namespace lightfields {

/// A simple representation of Gaussian Probability Distribution Function.
/// Based on Bromiley, Paul. "Products and convolutions of Gaussian probability density functions." Tina-Vision Memo 3.4 (2003): 1.
class PDFGaussian {
	public:
		static PDFGaussian fromPeak(float mu, float confidence) {
			assert(confidence >= 0.0f);

			if(confidence > 0.0f)
				return PDFGaussian(mu, 1.0f / (confidence * sqrt(2.0f * M_PI)));
			return PDFGaussian(mu, std::numeric_limits<float>::infinity());
		}

		PDFGaussian(float mu, float sigma) : m_mu(mu), m_sigma(sigma) {
		}

		float p(float val) const {
			if(std::isinf(m_sigma))
				return 0.0f;

			return 1.0f / (m_sigma * sqrt(2.0f * M_PI)) * std::exp(-0.5f * pow((val - m_mu) / m_sigma, 2));
		}

		float sigma() const {
			return m_sigma;
		}

		float mu() const {
			return m_mu;
		}

	private:
		float m_mu, m_sigma;
};

PDFGaussian operator+(const PDFGaussian& p, float val) {
	return PDFGaussian(p.mu() + val, p.sigma());
}

PDFGaussian operator+(float val, const PDFGaussian& p) {
	return PDFGaussian(p.mu() + val, p.sigma());
}

PDFGaussian operator*(const PDFGaussian& p, float val) {
	return PDFGaussian(p.mu() * val, p.sigma() * val);
}

PDFGaussian operator*(float val, const PDFGaussian& p) {
	return PDFGaussian(p.mu() * val, p.sigma() * val);
}

PDFGaussian operator/(const PDFGaussian& p, float val) {
	return PDFGaussian(p.mu() / val, p.sigma() / val);
}

PDFGaussian operator+(const PDFGaussian& p1, const PDFGaussian& p2) {
	// return PDFGaussian(p1.mu() + p2.mu(), p1.sigma() + p2.sigma());
	return PDFGaussian(p1.mu() + p2.mu(), std::sqrt(std::pow(p1.sigma(), 2) + std::pow(p2.sigma(), 2)));
}

PDFGaussian operator*(const PDFGaussian& p1, const PDFGaussian& p2) {
	// using limits
	if(std::isnan(p2.sigma()))
		return p1;
	if(std::isnan(p1.sigma()))
		return p2;

	// using the actual equation
	return PDFGaussian(
			(p2.mu() * std::pow(p1.sigma(), 2) + p1.mu() * std::pow(p2.sigma(), 2)) /
			(std::pow(p1.sigma(), 2) + std::pow(p2.sigma(), 2)),
		std::sqrt(
			std::pow(p1.sigma(), 2) * std::pow(p2.sigma(), 2) /
			(std::pow(p1.sigma(), 2) + std::pow(p2.sigma(), 2))
		)
	);
}

}
