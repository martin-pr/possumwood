#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include <dependency_graph/state.h>

namespace possumwood {
namespace opencv {

dependency_graph::State inpaint(const std::vector<cv::Mat>& inputs,
                                const std::vector<cv::Mat>& masks,
                                std::vector<cv::Mat>& result);

}
}  // namespace possumwood
