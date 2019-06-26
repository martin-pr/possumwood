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
dependency_graph::InAttr<bool> a_clustering, a_adaptiveThreshold, a_normalizeImage, a_filterQuads, a_fastCheck;
dependency_graph::OutAttr<possumwood::opencv::CalibrationPattern> a_outPattern;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::CalibrationPattern& inPattern = data.get(a_inPattern);

	cv::Mat result;
	bool success = false;

	// chessboard pattern
	if(inPattern.type() == possumwood::opencv::CalibrationPattern::kChessboard) {
		int flags = 0;
		if(data.get(a_adaptiveThreshold))
			flags |= cv::CALIB_CB_ADAPTIVE_THRESH;
		if(data.get(a_normalizeImage))
			flags |= cv::CALIB_CB_NORMALIZE_IMAGE;
		if(data.get(a_filterQuads))
			flags |= cv::CALIB_CB_FILTER_QUADS;
		if(data.get(a_fastCheck))
			flags |= cv::CALIB_CB_FAST_CHECK;

		success = cv::findChessboardCorners(*data.get(a_frame),
			inPattern.size(), result, flags
		);
	}

	// circle patterns, symmetric or asymmetric
	else {
		int flags = 0;
		if(data.get(a_clustering))
			flags |= cv::CALIB_CB_CLUSTERING;

		if(inPattern.type() == possumwood::opencv::CalibrationPattern::kAsymmetricCirclesGrid)
			flags |= cv::CALIB_CB_ASYMMETRIC_GRID;
		else
			flags |= cv::CALIB_CB_SYMMETRIC_GRID;

		success = cv::findCirclesGrid(*data.get(a_frame),
			inPattern.size(), result, flags
		);
	}

	dependency_graph::State status;
	if(!success)
		status.addWarning("Pattern not found in the input image!");

	const possumwood::opencv::CalibrationPattern grid(result, inPattern.size(), success, inPattern.type(), data.get(a_frame).size());
	data.set(a_outPattern, grid);

	return status;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inPattern, "in_pattern", possumwood::opencv::CalibrationPattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_clustering, "circles_grid/clustering");
	meta.addAttribute(a_adaptiveThreshold, "chessboard/adaptive_threshold");
	meta.addAttribute(a_normalizeImage, "chessboard/normalize_image");
	meta.addAttribute(a_filterQuads, "chessboard/filter_quads");
	meta.addAttribute(a_fastCheck, "chessboard/fast_check");
	meta.addAttribute(a_outPattern, "out_pattern", possumwood::opencv::CalibrationPattern(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_frame, a_outPattern);
	meta.addInfluence(a_inPattern, a_outPattern);
	meta.addInfluence(a_clustering, a_outPattern);
	meta.addInfluence(a_adaptiveThreshold, a_outPattern);
	meta.addInfluence(a_normalizeImage, a_outPattern);
	meta.addInfluence(a_filterQuads, a_outPattern);
	meta.addInfluence(a_fastCheck, a_outPattern);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/calibration/find_pattern", init);

}
