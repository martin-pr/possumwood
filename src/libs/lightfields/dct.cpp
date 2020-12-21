#include "dct.h"

namespace lightfields {

namespace {

// float dctCoef(float n, unsigned k) {
// 	if(k == 0)
// 		return 1.0f /* / std::sqrt(2.0f) */;

// 	return std::cos(M_PI * (n + 0.5f) * (float)k);
// }

// float idctCoef(float n, unsigned k) {
// 	if(k == 0)
// 		return 1.0f /* / std::sqrt(2.0f) */;

// 	return std::cos(M_PI * n * ((float)k + 0.5f));
// }

Imath::V2f dctCoef(float n, float k) {
	return Imath::V2f(std::cos(M_PI * n * k), -std::sin(M_PI * n * k));
}

}  // namespace

DCT dct(const Samples& samples, unsigned xy_samples, unsigned uv_samples) {
	std::vector<std::vector<cv::Mat>> data(xy_samples, std::vector<cv::Mat>(xy_samples));
	for(auto& a : data)
		for(auto& b : a)
			b = cv::Mat::zeros(uv_samples, uv_samples, CV_32FC(6));

	// much efficient, wow
	std::array<std::size_t, 3> counters{0, 0, 0};
	for(const auto& s : samples) {
		const Imath::V2f xy = s.xy / samples.sensorSize();
		const Imath::V2f uv = s.uv / 2.0f + Imath::V2f(0.5f, 0.5f);

		if(xy.x >= 0.0f && xy.y >= 0.0f && xy.x <= 1.0f && xy.y <= 1.0f)
			if(uv.x >= 0.0f && uv.y >= 0.0f && uv.x <= 1.0f && uv.y <= 1.0f) {
				if(s.color == Samples::kRGB) {
					++counters[0];
					++counters[1];
					++counters[2];
				}
				else
					++counters[s.color];

				for(unsigned yi = 0; yi < xy_samples; ++yi)
					for(unsigned xi = 0; xi < xy_samples; ++xi)
						for(unsigned vi = 0; vi < uv_samples; ++vi)
							for(unsigned ui = 0; ui < uv_samples; ++ui) {
								const Imath::V2f coef =
								    dctCoef(xy.y, yi) * dctCoef(xy.x, xi) * dctCoef(uv.y, vi) * dctCoef(uv.x, ui);

								float* d = data[yi][xi].ptr<float>(vi, ui);

								if(s.color == Samples::kRGB)
									for(int c = 0; c < 3; ++c) {
										d[c * 2] += coef[0] * s.value[c];
										d[c * 2 + 1] += coef[1] * s.value[c];
									}
								else {
									d[s.color * 2] += coef[0] * s.value[s.color];
									d[s.color * 2 + 1] += coef[1] * s.value[s.color];
								}
							}
			}
	}

	for(unsigned yi = 0; yi < xy_samples; ++yi)
		for(unsigned xi = 0; xi < xy_samples; ++xi)
			for(unsigned vi = 0; vi < uv_samples; ++vi)
				for(unsigned ui = 0; ui < uv_samples; ++ui) {
					float* d = data[yi][xi].ptr<float>(vi, ui);
					for(int c = 0; c < 6; ++c)
						d[c] /= counters[c / 2];
				}

	DCT result;
	result.m_data = std::move(data);
	result.m_xySamples = xy_samples;
	result.m_uvSamples = uv_samples;
	return result;
}

std::array<float, 3> DCT::get(float x, float y, float u, float v) const {
	u = u / 2.0f + 0.5f;
	v = v / 2.0f + 0.5f;

	assert(x >= 0.0f && x <= 1.0f);
	assert(y >= 0.0f && y <= 1.0f);
	assert(u >= 0.0f && u <= 1.0f);
	assert(v >= 0.0f && v <= 1.0f);

	std::array<float, 3> result{0, 0, 0};

	for(unsigned yi = 0; yi < m_xySamples; ++yi)
		for(unsigned xi = 0; xi < m_xySamples; ++xi)
			for(unsigned vi = 0; vi < m_uvSamples; ++vi)
				for(unsigned ui = 0; ui < m_uvSamples; ++ui) {
					const Imath::V2f coef = dctCoef(y, yi) * dctCoef(x, xi) * dctCoef(v, vi) * dctCoef(u, ui);

					const float* d = m_data[yi][xi].ptr<float>(vi, ui);

					for(int c = 0; c < 3; ++c) {
						result[c] += coef[0] * d[c * 2];
						result[c] += coef[1] * d[c * 2 + 1];
					}
				}

	return result;
}

std::ostream& operator<<(std::ostream& out, const lightfields::DCT& dct) {
	// out << "dct:" << std::endl;
	// for(int y = 0; y <= 10; ++y) {
	// 	for(int x = 0; x <= 10; ++x) {
	// 		const auto& arr = dct.get((float)x / 10.0f, (float)y / 10.0f, 0, 0);
	// 		out << arr[0] << "," << arr[1] << "," << arr[2] << "  ";
	// 	}

	// 	out << std::endl;
	// }
	out << "fft";
	return out;
}

}  // namespace lightfields
