#pragma once

#include <nlohmann/json.hpp>

#include <opencv2/opencv.hpp>

namespace lightfields {

class Bayer {
  public:
	Bayer(const nlohmann::json& meta);

	// values correspond to pattern "offsets" as used in the decode() method
	enum MozaicType { kBG = 0, kGB = 1, kGR = 2, kRG = 3 };
	MozaicType mozaic() const;

	cv::Mat decode(const unsigned char* raw) const;

	enum Flags { kNone = 0, kCorrectGain = 1 };
	cv::Mat uint16ToFloatMat(const cv::Mat& m, const Flags& f) const;

  private:
	template <typename T>
	struct Value {
		Value();
		Value(const nlohmann::json& json);

		template <typename U>
		Value<T>& operator=(const Value<U>& val);

		T operator[](unsigned id) const;

		T r, gr, gb, b;
		T max, min;
	};

	int m_width, m_height;
	Value<float> m_black, m_white;
	MozaicType m_mozaic;
	Value<float> m_gain;
};

}  // namespace lightfields
