#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::InAttr<unsigned> a_offset;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	// open the capture device
	cv::VideoCapture cap(filename.filename().string());
	// seek to the requested frame
	cap.set(cv::CAP_PROP_POS_FRAMES, data.get(a_offset));

	// and get the frame
	cv::Mat frame;
	cap >> frame;

	data.set(a_frame, possumwood::opencv::Frame(frame));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"MP4 files (*.mp4)",
	}));
	meta.addAttribute(a_offset, "offset");
	meta.addAttribute(a_frame, "frame");

	meta.addInfluence(a_filename, a_frame);
	meta.addInfluence(a_offset, a_frame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/video_frame", init);

}
