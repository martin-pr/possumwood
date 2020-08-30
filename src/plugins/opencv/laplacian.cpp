#include "laplacian.h"

#include <tbb/parallel_for.h>

namespace possumwood {
namespace opencv {

namespace {

cv::Mat normalize(const cv::Mat& in) {
	cv::Mat result = in.clone();

	float sum = 0.0f;
	for(int y = 0; y < in.rows; ++y)
		for(int x = 0; x < in.cols; ++x)
			sum += std::abs(result.at<float>(y, x));

	for(int y = 0; y < in.rows; ++y)
		for(int x = 0; x < in.cols; ++x)
			result.at<float>(y, x) /= sum;

	return result;
}

static const cv::Mat kernel_4cross =
    normalize((cv::Mat_<float>(3, 3) << 0.0, -1.0, 0.0, -1.0, 4.0, -1.0, 0.0, -1.0, 0.0));

static const cv::Mat kernel_8uniform =
    normalize((cv::Mat_<float>(3, 3) << -1.0, -1.0, -1.0, -1.0, 8.0, -1.0, -1.0, -1.0, -1.0));

static const cv::Mat kernel_8shaped =
    normalize((cv::Mat_<float>(3, 3) << -1.0, -2.0, -1.0, -2.0, 12.0, -2.0, -1.0, -2.0, -1.0));

static const cv::Mat kernel_24 =
    normalize((cv::Mat_<float>(5, 5) << 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 2.0, -8.0, 2.0, 0.0, 1.0, -8.0, 20.0, -8.0, 1.0,
               0.0, 2.0, -8.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0));

const cv::Mat& kernel(LaplacianKernel k) {
	switch(k) {
		case k4Cross:
			return kernel_4cross;
		case k8Uniform:
			return kernel_8uniform;
		case k8Shaped:
			return kernel_8shaped;
		case k24Shaped:
			return kernel_24;

		default:
			assert(false);
			static const cv::Mat s_dummy;
			return s_dummy;
	};
}

void convolution(const cv::Mat& src, cv::Mat& tgt, const cv::Mat& kernel) {
	assert(src.rows == tgt.rows && src.cols == tgt.cols);
	assert(src.channels() == 1);
	assert(tgt.channels() == 1);
	assert(src.type() == CV_32FC1 && tgt.type() == CV_32FC1 && kernel.type() == CV_32FC1);

	tbb::parallel_for(0, src.rows, [&](int y) {
		for(int x = 0; x < src.cols; ++x) {
			float& target = tgt.at<float>(y, x);

			// convolution
			for(int yi = 0; yi < kernel.rows; ++yi) {
				for(int xi = 0; xi < kernel.cols; ++xi) {
					int ypos = y + yi - kernel.rows / 2;
					int xpos = x + xi - kernel.cols / 2;

					// handling of edges - "clip" (or "mirror", commented out for now)
					if(ypos < 0)
						// ypos = -ypos;
						ypos = 0;
					if(ypos >= src.rows)
						// ypos = (image.rows-1) - (ypos-image.rows);
						ypos = src.rows - 1;

					if(xpos < 0)
						// xpos = -xpos;
						xpos = 0;
					if(xpos >= src.cols)
						// xpos = (image.cols-1) - (xpos-image.cols);
						xpos = src.cols - 1;

					const float& k = kernel.at<float>(yi, xi);
					const float& source = src.at<float>(ypos, xpos);

					target += k * source;
				}
			}
		}
	});
}

}  // namespace

const std::vector<std::pair<std::string, int>>& laplacianKernels() {
	static const std::vector<std::pair<std::string, int>> s_kernels({
	    std::pair<std::string, int>("4_cross", k4Cross),
	    std::pair<std::string, int>("8_uniform", k8Uniform),
	    std::pair<std::string, int>("8_shaped", k8Shaped),
	    std::pair<std::string, int>("24_shaped", k24Shaped),
	});

	return s_kernels;
}

cv::Mat laplacian(const cv::Mat& src, LaplacianKernel k) {
	assert(src.depth() == CV_32F);

	cv::Mat result = cv::Mat::zeros(src.rows, src.cols, src.type());

	std::vector<cv::Mat> tmp_src(src.channels()), tmp_dest(src.channels());
	cv::split(src, tmp_src);
	cv::split(result, tmp_dest);

	for(int c = 0; c < src.channels(); ++c)
		convolution(tmp_src[c], tmp_dest[c], kernel(k));

	cv::merge(tmp_dest, result);

	return result;
}

}  // namespace opencv
}  // namespace possumwood
