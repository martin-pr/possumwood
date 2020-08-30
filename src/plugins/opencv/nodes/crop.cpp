#include <actions/traits.h>
#include <maths/io/vec2.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_from, a_to;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Imath::Vec2<unsigned>& from = data.get(a_from);
	const Imath::Vec2<unsigned>& to = data.get(a_to);

	cv::Mat out = cv::Mat(*data.get(a_inFrame), cv::Rect(from[0], from[1], to[0], to[1])).clone();
	data.set(a_outFrame, possumwood::opencv::Frame(out));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_from, "crop/from", Imath::Vec2<unsigned>(0, 0));
	meta.addAttribute(a_to, "crop/to", Imath::Vec2<unsigned>(100, 100));
	meta.addAttribute(a_outFrame, "out", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_from, a_outFrame);
	meta.addInfluence(a_to, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/crop", init);

}  // namespace
