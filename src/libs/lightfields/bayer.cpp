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

	m_width = meta["image"]["width"].get<int>();
	m_height = meta["image"]["height"].get<int>();
}

cv::Mat Bayer::decode(const unsigned char* raw) {
	cv::Mat result(m_width, m_height, CV_32F);

	tbb::parallel_for(0, m_width * m_height, [&](int i) {
		const uint16_t c1 = raw[i / 2 * 3];
		const uint16_t c2 = raw[i / 2 * 3 + 1];
		const uint16_t c3 = raw[i / 2 * 3 + 2];

		uint16_t val;
		if(i % 2 == 0)
			val = (c1 << 4) + (c2 >> 4);
		else
			val = ((c2 & 0x0f) << 8) + c3;

		const unsigned patternId = (i % m_width) % 2 + ((i / m_width) % 2) * 2;

		const float fval =
		    ((float)val - (float)m_black[patternId]) / ((float)(m_white[patternId] - m_black[patternId]));

		result.at<float>(i / m_width, i % m_width) = fval;
	});

	return result;
}

}  // namespace lightfields
