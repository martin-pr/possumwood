#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"
#include "calibration_pattern.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::InAttr<possumwood::opencv::CalibrationPattern> a_inPattern;
dependency_graph::InAttr<bool> a_clustering;
dependency_graph::OutAttr<possumwood::opencv::CalibrationPattern> a_outPattern;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::CalibrationPattern& inPattern = data.get(a_inPattern);

	int flags = 0;

	if(data.get(a_clustering))
		flags |= cv::CALIB_CB_CLUSTERING;

	if(inPattern.type() == possumwood::opencv::CalibrationPattern::kSymmetricGrid)
		flags |= cv::CALIB_CB_SYMMETRIC_GRID;
	else
		flags |= cv::CALIB_CB_ASYMMETRIC_GRID;

	cv::Mat result;
	const bool success = cv::findCirclesGrid(*data.get(a_frame),
		inPattern.size(), result, flags
	);

	dependency_graph::State status;
	if(!success)
		status.addWarning("Pattern not found in the input image!");

	const possumwood::opencv::CalibrationPattern grid(result, inPattern.size(), success, inPattern.type());
	data.set(a_outPattern, grid);

	return status;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inPattern, "in_pattern", possumwood::opencv::CalibrationPattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_clustering, "clustering");
	meta.addAttribute(a_outPattern, "out_pattern", possumwood::opencv::CalibrationPattern(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_frame, a_outPattern);
	meta.addInfluence(a_inPattern, a_outPattern);
	meta.addInfluence(a_clustering, a_outPattern);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/calibration/find_pattern", init);

}
