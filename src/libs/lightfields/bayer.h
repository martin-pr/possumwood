#pragma once

#include <nlohmann/json.hpp>

#include <opencv2/opencv.hpp>

namespace lightfields {

class Bayer {
  public:
	Bayer(const nlohmann::json& meta);

	struct Value {
		Value();
		Value(const nlohmann::json& json);

		uint16_t operator[](unsigned id) const;

		uint16_t b, gb, gr, r;
	};

	cv::Mat decode(const unsigned char* raw);

  private:
	int m_width, m_height;
	Value m_black, m_white;
};

}  // namespace lightfields
