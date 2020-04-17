#pragma once

#include <array>

#include <opencv2/opencv.hpp>

namespace lightfields {

/// Implementation of the SLIC superpixels algorithm.
/// Achanta, Radhakrishna, et al. "SLIC superpixels compared to state-of-the-art superpixel methods." IEEE transactions on pattern analysis and machine intelligence 34.11 (2012): 2274-2282.
/// Each step of the algorithm is explicitly exposed to allow for different wiring based on the use-case.
struct SlicSuperpixels {
	/// Representation of one center of a superpixel
	struct Center {
		Center();
		Center(const cv::Mat& m, int r, int c);

		Center& operator +=(const Center& c);
		Center& operator /=(int div);

		int row, col;
		std::array<int, 3> color;
	};

	static int initS(int rows, int cols, int pixelCount);
};

}
