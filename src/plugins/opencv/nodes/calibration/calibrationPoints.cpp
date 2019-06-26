#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "calibration_pattern.h"
#include "calibration_points.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::CalibrationPattern> a_inObjectPattern, a_inCameraPattern;

dependency_graph::InAttr<possumwood::opencv::CalibrationPoints> a_inPoints;
dependency_graph::OutAttr<possumwood::opencv::CalibrationPoints> a_outPoints;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::opencv::CalibrationPoints points = data.get(a_inPoints);

	const cv::Mat& objPattern = *data.get(a_inObjectPattern);
	const cv::Mat& camPattern = *data.get(a_inCameraPattern);

	if(objPattern.rows != camPattern.rows)
		throw std::runtime_error("Number of object points doesn't match number of camera points - " + std::to_string(objPattern.rows) + " vs " + std::to_string(camPattern.rows));

	if(objPattern.size() != camPattern.size())
		throw std::runtime_error("Object and camera pattern sizes don't match!");

	// if empty, initialise
	if(points.empty())
		points = possumwood::opencv::CalibrationPoints(data.get(a_inCameraPattern).imageSize());

	// not empty - check for consistency
	else {
		if(points.imageSize() != data.get(a_inCameraPattern).imageSize())
			throw std::runtime_error("Points image size is not the same as camera pattern image size!");
	}

	// copy all the points
	possumwood::opencv::CalibrationPoints::Layer layer;
	for(int r=0;r<objPattern.rows;++r) {
		cv::Vec3f obj(objPattern.ptr<float>(r)[0], objPattern.ptr<float>(r)[1], 0.0f);
		cv::Vec2f cam(camPattern.ptr<float>(r)[0], camPattern.ptr<float>(r)[1]);

		layer.add(obj, cam);
	}
	points.addLayer(layer);

	data.set(a_outPoints, points);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inCameraPattern, "camera_pattern", possumwood::opencv::CalibrationPattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inObjectPattern, "object_pattern", possumwood::opencv::CalibrationPattern(), possumwood::AttrFlags::kVertical);

	meta.addAttribute(a_inPoints, "in_points");
	meta.addAttribute(a_outPoints, "out_points");

	meta.addInfluence(a_inCameraPattern, a_outPoints);
	meta.addInfluence(a_inObjectPattern, a_outPoints);
	meta.addInfluence(a_inPoints, a_outPoints);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/calibration/calibration_points", init);

}
