#include "samples.h"

#include <tbb/parallel_for.h>

#include <cassert>

namespace lightfields {

Samples::Samples() {
}

Samples::~Samples() {
}

void Samples::offset(float uvOffset) {
	tbb::parallel_for(std::size_t(0), m_samples.size(), [&](std::size_t i) {
		auto& s = m_samples[i];
		s.xy += s.uv * uvOffset;
	});
}

void Samples::threshold(float uvThreshold) {
	auto tgt = m_samples.begin();
	auto src = m_samples.begin();

	uvThreshold *= uvThreshold;
	for(; src != m_samples.end(); ++src) {
		if(src->uv.length2() < uvThreshold) {
			*tgt = *src;
			++tgt;
		}
	}
	m_samples.resize(tgt - m_samples.begin());
}

void Samples::filterInvalid() {
	auto tgt = m_samples.begin();
	auto src = m_samples.begin();

	const float widthf = m_size[0];
	const float heightf = m_size[1];
	for(; src != m_samples.end(); ++src) {
		if(src->xy[0] >= 0.0f && src->xy[1] >= 0.0f && src->xy[0] < widthf && src->xy[1] < heightf) {
			*tgt = *src;
			++tgt;
		}
	}
	m_samples.resize(tgt - m_samples.begin());
}

void Samples::scale(float xy_scale) {
	const Imath::V2f center(m_size[0] / 2, m_size[1] / 2);

	tbb::parallel_for(std::size_t(0), m_samples.size(), [&](std::size_t i) {
		auto& sample = m_samples[i];
		sample.xy = (sample.xy - center) * xy_scale + center;
	});
}

Samples::const_iterator Samples::begin() const {
	return m_samples.begin();
}

Samples::const_iterator Samples::end() const {
	return m_samples.end();
}

std::size_t Samples::size() const {
	return m_samples.size();
}

bool Samples::empty() const {
	return m_samples.empty();
}

const Imath::V2i Samples::sensorSize() const {
	return m_size;
}

/////////

namespace {

template <int CV_TYPE>
struct AssignSample;

template <>
struct AssignSample<CV_32FC1> {
	static void assign(Samples::Sample& sample, std::size_t x, std::size_t y, const cv::Mat& m) {
		// hardcoded bayer pattern, for now - generates kRed, kGreen or kBlue only
		sample.color = Samples::Color((x % 2) + (y % 2));

		// and the value
		sample.value = Imath::V3f(0, 0, 0);
		sample.value[sample.color] = *m.ptr<float>(y, x);
	}
};

template <>
struct AssignSample<CV_32FC3> {
	static void assign(Samples::Sample& sample, std::size_t x, std::size_t y, const cv::Mat& m) {
		// hardcoded bayer pattern, for now - generates kRed, kGreen or kBlue only
		sample.color = Samples::kRGB;

		// and the value
		const float* ptr = m.ptr<float>(y, x);
		sample.value = Imath::V3f(ptr[0], ptr[1], ptr[2]);
	}
};

template <int CV_TYPE>
std::vector<Samples::Sample> makeSamples(const Pattern& pattern, const cv::Mat& m) {
	std::vector<Samples::Sample> result;
	result.resize(m.rows * m.cols);

	// assemble the samples
	tbb::parallel_for(0, m.rows, [&](int y) {
		for(int x = 0; x < m.cols; ++x) {
			auto& sample = result[y * m.cols + x];

			const Pattern::Sample coords = pattern.sample(Imath::V2i(x, y));

			// and UV coordinates, -1..1
			sample.uv = coords.offset;

			// target pixel position, normalized (0..1) - adding the UV offset to handle displacement
			sample.xy = coords.lensCenter;

			// and assign the value
			AssignSample<CV_TYPE>::assign(sample, x, y, m);
		}
	});

	return result;
}

}  // namespace

Samples Samples::fromPattern(const Pattern& pattern, const cv::Mat& m) {
	assert(m.rows == pattern.sensorResolution().y && m.cols == pattern.sensorResolution().x);
	assert(m.type() == CV_32FC1 || m.type() == CV_32FC3);

	// if(data.type() != CV_32FC1 && data.type() != CV_32FC3)
	// 	throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input.");

	Samples result;

	result.m_size = pattern.sensorResolution();

	switch(m.type()) {
		case CV_32FC1:
			result.m_samples = makeSamples<CV_32FC1>(pattern, m);
			break;

		case CV_32FC3:
			result.m_samples = makeSamples<CV_32FC3>(pattern, m);
			break;
	}

	return result;
}

/////////

std::ostream& operator<<(std::ostream& out, const Samples& f) {
	out << "(lightfield samples - " << f.size() << " samples)";

	return out;
}

}  // namespace lightfields
