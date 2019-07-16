#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "lightfield_pattern.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::opencv::LightfieldPattern> a_pattern;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::LightfieldPattern& pattern = data.get(a_pattern);

	if((*data.get(a_in)).type() != CV_8UC3)
		throw std::runtime_error("Only 8-bit RGB format supported, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	cv::Mat mat = (*data.get(a_in)).clone();

	for(int y=0;y<mat.rows;++y)
		for(int x=0;x<mat.cols;++x) {
			const cv::Vec4f value = pattern.sample(cv::Vec2i(x, y));

			unsigned char* color = mat.ptr<unsigned char>(y, x);
			float current = value[2]*value[2] + value[3]*value[3];
			// if(current > 1.0)
			// 	color[2] += (current - 1.0) * 250.0;
			if(current < 1.0)
				color[2] += (1.0 - current) * 128 + 127;
		}

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pattern, "pattern", possumwood::opencv::LightfieldPattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_pattern, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/draw_pattern", init);

}
