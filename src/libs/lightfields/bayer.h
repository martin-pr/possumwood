#pragma once

#include <nlohmann/json.hpp>

#include <opencv2/opencv.hpp>

namespace lightfields {

class Bayer {
  public:
	enum Decoding { kNone = 0, kBasic, kEA };

	Bayer(const nlohmann::json& meta);

	struct Value {
		Value();
		Value(const nlohmann::json& json);

		uint16_t operator[](unsigned id) const;

		uint16_t b, gb, gr, r;
	};

	cv::Mat decode(const unsigned char* raw, Decoding decoding);

  private:
	int m_width, m_height;
	Value m_black, m_white;
	std::size_t m_opencvBayerEnumOffset, m_bayerOffset;
};

}  // namespace lightfields
