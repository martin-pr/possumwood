#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

namespace possumwood {
namespace opencv {

enum LaplacianKernel { k4Cross, k8Uniform, k8Shaped, k24Shaped };

const std::vector<std::pair<std::string, int>>& laplacianKernels();

cv::Mat laplacian(const cv::Mat& src, LaplacianKernel kernel);

}  // namespace opencv
}  // namespace possumwood
