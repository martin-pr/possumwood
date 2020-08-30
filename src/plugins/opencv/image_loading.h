#pragma once

#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>

#include "exif.h"

namespace possumwood {
namespace opencv {

std::pair<cv::Mat, Exif> load(const boost::filesystem::path& filename);

}
}  // namespace possumwood
