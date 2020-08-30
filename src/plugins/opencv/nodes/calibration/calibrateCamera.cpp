#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "calibration_points.h"
#include "camera_intrinsics.h"
#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::CalibrationPoints> a_points;
dependency_graph::OutAttr<possumwood::opencv::CameraIntrinsics> a_camIntrinsics;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::CalibrationPoints& calibrationPoints = data.get(a_points);

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	std::vector<cv::Mat> rvecs, tvecs;
	cv::Mat distCoeffs;

	cv::calibrateCamera(calibrationPoints.objectPoints(), calibrationPoints.cameraPoints(),
	                    calibrationPoints.imageSize(), cameraMatrix, distCoeffs, rvecs, tvecs);

	possumwood::opencv::CameraIntrinsics intrinsics(cameraMatrix, distCoeffs, calibrationPoints.imageSize());
	data.set(a_camIntrinsics, intrinsics);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_points, "points");
	meta.addAttribute(a_camIntrinsics, "camera_intrinsics");

	meta.addInfluence(a_points, a_camIntrinsics);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/calibration/calibrate_camera", init);

}  // namespace
