#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	data.set(a_frame, possumwood::opencv::Frame(cv::imread(filename.filename().string())));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"Image files (*.png *.jpg *.jpe *.jpeg)",
	}));
	meta.addAttribute(a_frame, "frame");

	meta.addInfluence(a_filename, a_frame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/image", init);

}
