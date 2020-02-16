#include <possumwood_sdk/node_implementation.h>

#include <lightfields/depth.h>
#include <lightfields/nearest_integration.h>

#include <maths/io/vec2.h>

#include "lightfields.h"
#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_res;
dependency_graph::InAttr<float> a_start, a_end;
dependency_graph::InAttr<unsigned> a_steps;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Samples& samples = data.get(a_samples);

	const cv::Mat in = *data.get(a_in);
	if(in.type() != CV_32FC1 && in.type() != CV_32FC3)
		throw std::runtime_error("Expected a 1 or 3 channel float input image");

	if(data.get(a_steps) < 1)
		throw std::runtime_error("2 or more quantisation steps are required for the depth algorithm");

	lightfields::Depth depth;

	for(unsigned a=0;a<data.get(a_steps);++a) {
		const float w = (float)a / (float)(data.get(a_steps)-1);
		const float d = data.get(a_start) + (data.get(a_end) - data.get(a_start)) * w;

		auto tmp = lightfields::nearest::integrate(samples, data.get(a_res), in, d);
		auto correspondence = lightfields::nearest::correspondence(samples, in, tmp, d);

		std::vector<cv::Mat> split(3);
		cv::split(correspondence, split);

		for(auto& s : split)
			depth.addLayer(d, s);
	}

	data.set(a_out, possumwood::opencv::Frame(depth.eval()));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_image", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_res, "resolution", Imath::Vec2<unsigned>(300u, 300u));
	meta.addAttribute(a_start, "samples/start", -40.0f);
	meta.addAttribute(a_end, "samples/end", 40.0f);
	meta.addAttribute(a_steps, "samples/steps", 9u);
	meta.addAttribute(a_out, "out_image", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_res, a_out);
	meta.addInfluence(a_start, a_out);
	meta.addInfluence(a_end, a_out);
	meta.addInfluence(a_steps, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/depth", init);

}
