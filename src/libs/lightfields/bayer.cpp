#include "bayer.h"

#include <tbb/parallel_for.h>

namespace lightfields {

namespace {

uint16_t getComponent(const nlohmann::json& json, const std::string& key) {
	auto it = json.find(key);
	if(it == json.end())
		throw std::runtime_error("Error reading Bayer component - missing '" + key + "' value.");
	return it->get<uint16_t>();
}

}  // namespace

Bayer::Value::Value() : b(0), gb(0), gr(0), r(0) {
}

Bayer::Value::Value(const nlohmann::json& json) {
	b = getComponent(json, "b");
	gb = getComponent(json, "gb");
	gr = getComponent(json, "gr");
	r = getComponent(json, "r");
}

uint16_t Bayer::Value::operator[](unsigned id) const {
	assert(id < 4 && "Bayer value ID should be in range 0..3");
	return reinterpret_cast<const uint16_t*>(this)[id];
}

Bayer::Bayer(const nlohmann::json& meta) {
	m_black = lightfields::Bayer::Value(meta["image"]["rawDetails"]["pixelFormat"]["black"]);
	m_white = lightfields::Bayer::Value(meta["image"]["rawDetails"]["pixelFormat"]["white"]);

	const std::string upperLeftPixel = meta["devices"]["sensor"]["mosaic"]["upperLeftPixel"].get<std::string>();

	if(upperLeftPixel == "b") {
		m_opencvBayerEnumOffset = 2;
		m_bayerOffset = 0;
	}
	else if(upperLeftPixel == "gb") {
		m_opencvBayerEnumOffset = 3;
		m_bayerOffset = 1;
	}
	else if(upperLeftPixel == "gr") {
		m_opencvBayerEnumOffset = 1;
		m_bayerOffset = 2;
	}
	else if(upperLeftPixel == "r") {
		m_opencvBayerEnumOffset = 0;
		m_bayerOffset = 3;
	}
	else
		throw std::runtime_error("Unknown 'upperLeftPixel' value '" + upperLeftPixel + "' in image metadata");

	m_width = meta["image"]["width"].get<int>();
	m_height = meta["image"]["height"].get<int>();

	if(meta["devices"]["sensor"]["bitsPerPixel"].get<int>() != 12)
		throw std::runtime_error("Only 12 bits per pixel supported at the moment");
}

cv::Mat Bayer::decode(const unsigned char* raw, Decoding decoding) {
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

		const unsigned patternId = ((i % m_width) % 2 + ((i / m_width) % 2) * 2 + m_bayerOffset) % 4;

		if(val < m_black[patternId])
			val = m_black[patternId];
		if(val > m_white[patternId])
			val = m_white[patternId];

		const float fval =
		    ((float)val - (float)m_black[patternId]) / ((float)(m_white[patternId] - m_black[patternId]));

		// int values normalized between 0 and 12-bit max
		tmp.at<uint16_t>(i / m_width, i % m_width) = floor((float)((1 << 12) - 1) * fval);
	});

	if(decoding == kNone)
		return tmp;
	else {
		cv::Mat result(m_width, m_height, CV_16UC3);

		int mode;
		switch(decoding) {
			case kBasic:
				mode = cv::COLOR_BayerBG2BGR + m_opencvBayerEnumOffset;
				break;

			case kEA:
				mode = cv::COLOR_BayerBG2BGR_EA + m_opencvBayerEnumOffset;
				break;

			default:
				throw std::runtime_error("Unsupported debayer mode");
		}

		cvtColor(tmp, result, mode);

		return result;
	}
}

}  // namespace lightfields
