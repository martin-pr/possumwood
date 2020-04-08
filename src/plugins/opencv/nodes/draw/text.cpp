#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>

#include <maths/io/vec2.h>
#include <maths/io/vec3.h>

#include "frame.h"

namespace {

static const std::vector<std::pair<std::string, int>> s_font {
	{"FONT_HERSHEY_SIMPLEX", cv::FONT_HERSHEY_SIMPLEX},
	{"FONT_HERSHEY_PLAIN", cv::FONT_HERSHEY_PLAIN},
	{"FONT_HERSHEY_DUPLEX", cv::FONT_HERSHEY_DUPLEX},
	{"FONT_HERSHEY_COMPLEX", cv::FONT_HERSHEY_COMPLEX},
	{"FONT_HERSHEY_TRIPLEX", cv::FONT_HERSHEY_TRIPLEX},
	{"FONT_HERSHEY_COMPLEX_SMALL", cv::FONT_HERSHEY_COMPLEX_SMALL},
	{"FONT_HERSHEY_SCRIPT_SIMPLEX", cv::FONT_HERSHEY_SCRIPT_SIMPLEX},
	{"FONT_HERSHEY_SCRIPT_COMPLEX", cv::FONT_HERSHEY_SCRIPT_COMPLEX},
};


dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<std::string> a_text;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_pos;
dependency_graph::InAttr<Imath::Vec3<unsigned>> a_color;
dependency_graph::InAttr<possumwood::Enum> a_font;
dependency_graph::InAttr<float> a_fontScale;
dependency_graph::InAttr<int> a_thickness;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = (*data.get(a_inFrame)).clone();

	cv::putText(result, data.get(a_text), cv::Point(data.get(a_pos)[0], data.get(a_pos)[1]), data.get(a_font).intValue(),
		data.get(a_fontScale), cv::Scalar(data.get(a_color)[0], data.get(a_color)[1], data.get(a_color)[2]), data.get(a_thickness));

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_text, "text", std::string("Text"));
	meta.addAttribute(a_pos, "pos", Imath::Vec2<unsigned>(0, 20));
	meta.addAttribute(a_font, "font/face", possumwood::Enum(s_font.begin(), s_font.end()));
	meta.addAttribute(a_fontScale, "font/scale", 1.0f);
	meta.addAttribute(a_color, "style/color", Imath::Vec3<unsigned>(255, 255, 255));
	meta.addAttribute(a_thickness, "style/thickness", 1);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_text, a_outFrame);
	meta.addInfluence(a_pos, a_outFrame);
	meta.addInfluence(a_color, a_outFrame);
	meta.addInfluence(a_font, a_outFrame);
	meta.addInfluence(a_fontScale, a_outFrame);
	meta.addInfluence(a_thickness, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/draw/text", init);

}
