#include <lightfields/gaussian_integration.h>
#include <lightfields/nearest_integration.h>
#include <maths/io/vec2.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include "lightfields.h"
#include "sequence.h"

namespace {

dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_res;
dependency_graph::InAttr<float> a_start, a_end;
dependency_graph::InAttr<unsigned> a_steps;
dependency_graph::InAttr<possumwood::Enum> a_method;
dependency_graph::InAttr<float> a_sigma;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_out, a_corresp;

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Samples& samples = data.get(a_samples);

	if(data.get(a_steps) < 1)
		throw std::runtime_error("2 or more quantisation steps are required for the depth algorithm");

	if(data.get(a_start) >= data.get(a_end))
		throw std::runtime_error("Invalid interval - start needs to be lower than end");

	possumwood::opencv::Sequence out(data.get(a_steps));
	possumwood::opencv::Sequence corresp(data.get(a_steps));

	tbb::parallel_for(0u, data.get(a_steps), [&](unsigned a) {
		const float w = (float)a / (float)(data.get(a_steps) - 1);
		const float d = data.get(a_start) + (data.get(a_end) - data.get(a_start)) * w;

		if(data.get(a_method).intValue() == 0) {
			auto tmp = lightfields::nearest::integrate(samples, data.get(a_res), d);
			corresp(a) = lightfields::nearest::correspondence(samples, tmp, d);

			out(a) = std::move(tmp.average);
		}
		else {
			auto tmp = lightfields::gaussian::integrate(samples, data.get(a_res), data.get(a_sigma), d);
			corresp(a) = lightfields::gaussian::correspondence(samples, tmp, data.get(a_sigma), d);

			out(a) = std::move(tmp.average);
		}
	});

	data.set(a_out, out);
	data.set(a_corresp, corresp);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_res, "resolution", Imath::Vec2<unsigned>(300u, 300u));
	meta.addAttribute(a_start, "samples/start", -40.0f);
	meta.addAttribute(a_end, "samples/end", 40.0f);
	meta.addAttribute(a_steps, "samples/steps", 9u);
	meta.addAttribute(a_method, "integration/method", possumwood::Enum({"Nearest neighbour", "Gaussian splatting"}));
	meta.addAttribute(a_sigma, "integration/sigma", 3.0f);
	meta.addAttribute(a_out, "out_seq");
	meta.addAttribute(a_corresp, "corresp_seq");

	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_res, a_out);
	meta.addInfluence(a_start, a_out);
	meta.addInfluence(a_end, a_out);
	meta.addInfluence(a_steps, a_out);
	meta.addInfluence(a_method, a_out);
	meta.addInfluence(a_sigma, a_out);

	meta.addInfluence(a_samples, a_corresp);
	meta.addInfluence(a_res, a_corresp);
	meta.addInfluence(a_start, a_corresp);
	meta.addInfluence(a_end, a_corresp);
	meta.addInfluence(a_steps, a_corresp);
	meta.addInfluence(a_method, a_out);
	meta.addInfluence(a_sigma, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/depth", init);

}  // namespace
