#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>
#include <tbb/parallel_sort.h>

#include "bspline.inl"
#include "datatypes/bspline.h"
#include "lightfields.h"

namespace {

dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::InAttr<unsigned> a_xyRes, a_uvRes;
dependency_graph::OutAttr<std::array<possumwood::opencv::BSpline<4>, 3>> a_bspline;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State state;

	std::array<possumwood::opencv::BSpline<4>, 3> bspline;
	for(int a = 0; a < 3; ++a)
		bspline[a] = possumwood::opencv::BSpline<4>(
		    std::array<std::size_t, 4>{{data.get(a_xyRes), data.get(a_xyRes), data.get(a_uvRes), data.get(a_uvRes)}});

	lightfields::Samples samples = data.get(a_samples);

	if(!samples.empty()) {
		const Imath::V2f xy_scale = Imath::V2f(data.get(a_xyRes) - 1, data.get(a_xyRes) - 1) /
		                            Imath::V2f(samples.sensorSize().x - 1, samples.sensorSize().y - 1);
		const Imath::V2f uv_scale = Imath::V2f(data.get(a_uvRes) - 1, data.get(a_uvRes) - 1);

		const float xy_lim = data.get(a_xyRes);
		const float uv_lim = data.get(a_uvRes);

		std::atomic<std::size_t> totalMissed(0);

		// sort the samples based on Y values
		tbb::parallel_sort(samples.begin(), samples.end(),
		                   [](const lightfields::Samples::Sample& s1, const lightfields::Samples::Sample& s2) {
			                   return s1.xy.y < s2.xy.y;
		                   });

		// get the range for each Y "line"
		std::vector<lightfields::Samples::const_iterator> lineStarts;
		int current = 0;
		for(auto it = samples.begin(); it != samples.end(); ++it)
			if(lineStarts.empty() || floor(it->xy.y) != current) {
				lineStarts.push_back(it);
				current = floor(it->xy.y);
			}
		lineStarts.push_back(samples.end());

		// process samples in pairs - every second pair to be skipped in each phase
		for(std::size_t phase = 0; phase < 2; ++phase) {
			tbb::parallel_for(std::size_t(0), lineStarts.size() / 4 + 1, [&](std::size_t index) {
				std::size_t missed = 0;
				for(std::size_t ofs = 0; ofs < 2; ++ofs) {
					const std::size_t y = index * 4 + 2 * phase + ofs;
					if(y < lineStarts.size() - 1) {
						for(auto it = lineStarts[y]; it != lineStarts[y + 1]; ++it) {
							auto& s = *it;

							const Imath::V2f xy = s.xy * xy_scale;
							const Imath::V2f uv = (s.uv / 2.0 + Imath::V2f(0.5f, 0.5f)) * uv_scale;

							if(xy.x >= 0.0f && xy.x <= xy_lim && xy.y >= 0.0f && xy.y <= xy_lim && uv.x >= 0.0f &&
							   uv.x <= uv_lim && uv.y >= 0.0f && uv.y <= uv_lim) {
								if(s.color == lightfields::Samples::kRGB)
									for(int a = 0; a < 3; ++a)
										bspline[a].addSample(std::array<float, 4>{xy.x, xy.y, uv.x, uv.y}, s.value[a]);
								else
									bspline[s.color].addSample(std::array<float, 4>{xy.x, xy.y, uv.x, uv.y},
									                           s.value[s.color]);
							}
							else
								++missed;
						}
					}

					totalMissed += missed;
				}
			});
		}
		if(totalMissed > 0)
			state.addWarning(std::to_string(totalMissed) + " samples were missed when constructing the b spline.");
	}

	data.set(a_bspline, bspline);

	return state;
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
