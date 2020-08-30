#include <actions/traits.h>
#include <maths/io/vec3.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "rectangles.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<std::vector<cv::Rect>> a_features;
dependency_graph::InAttr<Imath::V3f> a_color;
dependency_graph::InAttr<unsigned> a_thickness;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat frame = (*data.get(a_inFrame)).clone();

	const cv::Scalar color(data.get(a_color).x, data.get(a_color).y, data.get(a_color).z);

	for(auto& r : data.get(a_features))
		cv::rectangle(frame, r, color, (int)data.get(a_thickness));

	data.set(a_outFrame, possumwood::opencv::Frame(frame));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_features, "features", std::vector<cv::Rect>(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_color, "draw/color", Imath::V3f(0, 0, 255));
	meta.addAttribute(a_thickness, "draw/thickness", 3u);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_features, a_outFrame);
	meta.addInfluence(a_color, a_outFrame);
	meta.addInfluence(a_thickness, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/clasifier/draw_features", init);

}  // namespace
