#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include "bspline.inl"
#include "datatypes/bspline.h"
#include "lightfields.h"

namespace {

dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::InAttr<unsigned> a_xyRes, a_uvRes;
dependency_graph::OutAttr<std::array<possumwood::opencv::BSpline<4>, 3>> a_bspline;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::array<possumwood::opencv::BSpline<4>, 3> bspline;
	for(int a = 0; a < 3; ++a)
		bspline[a] = possumwood::opencv::BSpline<4>(
		    std::array<std::size_t, 4>{{data.get(a_xyRes), data.get(a_xyRes), data.get(a_uvRes), data.get(a_uvRes)}});

	const lightfields::Samples& samples = data.get(a_samples);
	for(auto& s : samples) {
		const Imath::V2f xy = s.xy / Imath::V2f(samples.sensorSize());
		const Imath::V2f uv = s.uv / 2.0 + Imath::V2f(0.5f, 0.5f);

		if(xy.x >= 0.0f && xy.x <= 1.0f && xy.y >= 0.0f && xy.y <= 1.0f && uv.x >= 0.0f && uv.x <= 1.0f &&
		   uv.y >= 0.0f && uv.y <= 1.0f) {
			if(s.color == lightfields::Samples::kRGB)
				for(int a = 0; a < 3; ++a)
					bspline[a].addSample(std::array<float, 4>{xy.x, xy.y, uv.x, uv.y}, s.value[a]);
			else
				bspline[s.color].addSample(std::array<float, 4>{xy.x, xy.y, uv.x, uv.y}, s.value[s.color]);
		}
	}

	data.set(a_bspline, bspline);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_xyRes, "resolution/xy", 10u);
	meta.addAttribute(a_uvRes, "resolution/uv", 10u);
	meta.addAttribute(a_bspline, "bspline", std::array<possumwood::opencv::BSpline<4>, 3>(),
	                  possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_samples, a_bspline);
	meta.addInfluence(a_xyRes, a_bspline);
	meta.addInfluence(a_uvRes, a_bspline);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/samples_to_bspline", init);

}  // namespace
