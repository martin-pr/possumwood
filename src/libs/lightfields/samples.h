#pragma once

#include <iostream>
#include <limits>
#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>

#include "pattern.h"

namespace lightfields {

/// Creates 2D samples for lightfield refocusing (i.e., sampling a 2D plane in 4D space with constant U and V).
class Samples {
  public:
	enum Color { kRGB = 3, kRed = 2, kGreen = 1, kBlue = 0 };

	struct Sample {
		Imath::V2f xy;     ///< XY coordinates of the sample
		Imath::V2f uv;     ///< UV coordinates of the sample
		Color color;       ///< Target colour channel, based on the Bayer pattern, or RGB for de-Bayer data
		Imath::V3f value;  ///< Sample colour value (only the valid channels are populated)
	};

	Samples(const Imath::V2i& sensorSize = Imath::V2i(0, 0));
	~Samples();

	Samples(const Samples&) = default;
	Samples& operator=(const Samples&) = default;

	Samples(Samples&&) = default;
	Samples& operator=(Samples&&) = default;

	using const_iterator = std::vector<Sample>::const_iterator;
	using iterator = std::vector<Sample>::iterator;

	const_iterator begin() const;
	const_iterator end() const;

	iterator begin();
	iterator end();

	std::size_t size() const;
	bool empty() const;

	const Imath::V2i sensorSize() const;

	void offset(float uvOffset);
	void threshold(float uvThreshold);
	void scale(float xy_scale);
	void filterInvalid();

	static Samples fromPattern(const Pattern& p, const cv::Mat& data);

	void resize(std::size_t size);
	Sample& operator[](std::size_t index);
	const Sample& operator[](std::size_t index) const;

  private:
	std::vector<Sample> m_samples;
	Imath::V2i m_size;
};

std::ostream& operator<<(std::ostream& out, const Samples& f);

}  // namespace lightfields
