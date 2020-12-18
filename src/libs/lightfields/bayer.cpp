#include "bayer.h"

#include <tbb/parallel_for.h>

namespace lightfields {

namespace {

template <typename T>
T getComponent(const nlohmann::json& json, const std::string& key) {
	auto it = json.find(key);
	if(it == json.end())
		throw std::runtime_error("Error reading Bayer component - missing '" + key + "' value.");
	return it->get<T>();
}

}  // namespace

template <typename T>
Bayer::Value<T>::Value() : r(0), gr(0), gb(0), b(0) {
}

template <typename T>
Bayer::Value<T>::Value(const nlohmann::json& json) {
	b = getComponent<T>(json, "b");
	gb = getComponent<T>(json, "gb");
	gr = getComponent<T>(json, "gr");
	r = getComponent<T>(json, "r");

	max = std::max(std::max(b, gb), std::max(gr, r));
	min = std::min(std::min(b, gb), std::min(gr, r));
}

template <typename T>
T Bayer::Value<T>::operator[](unsigned id) const {
	assert(id < 4 && "Bayer value ID should be in range 0..3");
	return reinterpret_cast<const T*>(this)[id];
}

template <typename T>
template <typename U>
Bayer::Value<T>& Bayer::Value<T>::operator=(const Value<U>& val) {
	b = val.b;
	gb = val.gb;
	gr = val.gr;
	r = val.r;

	max = val.max;
	min = val.min;

	return *this;
}

Bayer::Bayer(const nlohmann::json& meta) {
	m_black = lightfields::Bayer::Value<uint16_t>(meta["image"]["rawDetails"]["pixelFormat"]["black"]);
	m_white = lightfields::Bayer::Value<uint16_t>(meta["image"]["rawDetails"]["pixelFormat"]["white"]);

	const std::string upperLeftPixel = meta["devices"]["sensor"]["mosaic"]["upperLeftPixel"].get<std::string>();

	if(upperLeftPixel == "b") {
		m_mozaic = kBG;
	}
	else if(upperLeftPixel == "gb") {
		m_mozaic = kGB;
	}
	else if(upperLeftPixel == "gr") {
		m_mozaic = kGR;
	}
	else if(upperLeftPixel == "r") {
		m_mozaic = kRG;
	}
	else
		throw std::runtime_error("Unknown 'upperLeftPixel' value '" + upperLeftPixel + "' in image metadata");

	m_width = meta["image"]["width"].get<int>();
	m_height = meta["image"]["height"].get<int>();

	if(meta["devices"]["sensor"]["bitsPerPixel"].get<int>() != 12)
		throw std::runtime_error("Only 12 bits per pixel supported at the moment");

	m_gain = lightfields::Bayer::Value<float>(meta["devices"]["sensor"]["analogGain"]);
}

Bayer::MozaicType Bayer::mozaic() const {
	return m_mozaic;
}

cv::Mat Bayer::decode(const unsigned char* raw) const {
	cv::Mat tmp(m_width, m_height, CV_16UC1);

	tbb::parallel_for(0, m_width * m_height, [&](int i) {
		const uint16_t c1 = raw[i / 2 * 3];
		const uint16_t c2 = raw[i / 2 * 3 + 1];
		const uint16_t c3 = raw[i / 2 * 3 + 2];

		uint16_t val;
		if(i % 2 == 0)
			val = (c1 << 4) + (c2 >> 4);
		else
			val = ((c2 & 0x0f) << 8) + c3;

		tmp.at<uint16_t>(i / m_width, i % m_width) = val;
	});

	return tmp;
}

cv::Mat Bayer::uint16ToFloatMat(const cv::Mat& m, const Flags& f) const {
	cv::Mat result = cv::Mat::zeros(m.rows, m.cols, CV_MAKETYPE(CV_32F, m.channels()));

	tbb::parallel_for(tbb::blocked_range<int>(0, m_width * m_height),
	                  [&result, &m, this, &f](const tbb::blocked_range<int>& range) {
		                  for(int i = range.begin(); i != range.end(); ++i) {
			                  const uint16_t* val = m.ptr<uint16_t>(i / m_width, i % m_width);

			                  for(int chan = 0; chan < m.channels(); ++chan) {
				                  float fval = val[chan];

				                  // for 1-channel pics use bayer; 3-channel use the values directly
				                  unsigned patternId;
				                  if(m.channels() == 1)
					                  patternId = ((i % m_width) % 2 + ((i / m_width) % 2) * 2 + (int)m_mozaic) % 4;
				                  else {
					                  switch(chan) {
						                  case 0:
							                  patternId = 0;  // blue
							                  break;
						                  case 1:
							                  patternId = 1;  // green; assuming GB == GR
							                  break;
						                  case 2:
							                  patternId = 3;  // red
							                  break;
					                  }
				                  }

				                  // adjust based on normalized gain
				                  // This is tricky - gain adjustment introduces "purple edges" in oversatureated
				                  // regions. Needs more work.
				                  if(f & kCorrectGain)
					                  fval *= m_gain[patternId] / m_gain.min;

				                  // make sure the value is in black-white range to avoid negative values
				                  if(fval < m_black[patternId])
					                  fval = m_black[patternId];
				                  if(fval > m_white[patternId])
					                  fval = m_white[patternId];

				                  // normalize to 0..1 for black..white
				                  fval = (fval - m_black[patternId]) / (m_white[patternId] - m_black[patternId]);

				                  // and set the output value
				                  result.ptr<float>(i / m_width, i % m_width)[chan] = fval;
			                  }
		                  }
	                  });

	return result;
}

}  // namespace lightfields
