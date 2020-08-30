#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "camera_intrinsics.h"
#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<possumwood::opencv::CameraIntrinsics> a_camIntrinsics;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State status;

	const possumwood::opencv::CameraIntrinsics& intrinsics = data.get(a_camIntrinsics);
	const possumwood::opencv::Frame& inFrame = data.get(a_inFrame);

	if(inFrame.size() != intrinsics.imageSize()) {
		std::stringstream ss;
		ss << "Provided intrinsics expect a different image size! Intrinics expect " << intrinsics.imageSize().width
		   << "x" << intrinsics.imageSize().height << " while image is of size " << inFrame.size().width << "x"
		   << inFrame.size().height << ".";

		status.addWarning(ss.str());
	}

	cv::Mat undistorted;
	cv::undistort(*inFrame, undistorted, intrinsics.matrix(), intrinsics.distCoeffs());

	data.set(a_outFrame, possumwood::opencv::Frame(undistorted));

	return status;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_camIntrinsics, "intrinsics");
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_camIntrinsics, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/calibration/undistort", init);

}  // namespace
