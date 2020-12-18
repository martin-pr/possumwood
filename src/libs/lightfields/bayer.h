#pragma once

#include <nlohmann/json.hpp>

#include <opencv2/opencv.hpp>

namespace lightfields {

class Bayer {
  public:
	enum Decoding { kNone = 0, kBasic, kEA };

	Bayer(const nlohmann::json& meta);

	cv::Mat decode(const unsigned char* raw, Decoding decoding);

  private:
	template <typename T>
	struct Value {
		Value();
		Value(const nlohmann::json& json);

		template <typename U>
		Value<T>& operator=(const Value<U>& val);

		T operator[](unsigned id) const;

		T b, gb, gr, r;
		T max, min;
	};

	int m_width, m_height;
	Value<float> m_black, m_white;
	std::size_t m_opencvBayerEnumOffset, m_bayerOffset;
	Value<float> m_gain;
};

}  // namespace lightfields
