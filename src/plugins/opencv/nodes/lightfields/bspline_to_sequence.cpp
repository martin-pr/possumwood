#include <possumwood_sdk/node_implementation.h>

#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>

#include <maths/io/vec2.h>

#include "datatypes/bspline.h"

#include "bspline.inl"
#include "sequence.h"

namespace {

dependency_graph::InAttr<std::array<possumwood::opencv::BSpline<4>, 3>> a_bspline;
dependency_graph::InAttr<Imath::V2i> a_xyResolution, a_uvResolution;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_sequence;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Imath::V2i& xy_res = data.get(a_xyResolution);
	const Imath::V2i& uv_res = data.get(a_uvResolution);

	if(uv_res.x < 1 || uv_res.y < 1)
		throw std::runtime_error("UV resolution needs to be >= 1");

	const std::array<possumwood::opencv::BSpline<4>, 3>& bspline = data.get(a_bspline);

	possumwood::opencv::Sequence seq;

	tbb::parallel_for(0, uv_res.y, [&](int v) {
		tbb::parallel_for(0, uv_res.x, [&](int u) {
			Imath::V2f uv;
			if(uv_res.x > 1)
				uv.x = float(u) / (uv_res.x - 1) * float(bspline[0].size(2) - 1);
			else
				uv.x = float(bspline[0].size(2) - 1) / 2.0f;

			if(uv_res.y > 1)
				uv.y = float(v) / (uv_res.y - 1) * float(bspline[0].size(3) - 1);
			else
				uv.y = float(bspline[0].size(3) - 1) / 2.0f;

			Imath::V2f xy_scale =
			    Imath::V2f(bspline[0].size(0) - 1, bspline[0].size(1) - 1) / Imath::V2f(data.get(a_xyResolution));

			cv::Mat m = cv::Mat::zeros(xy_res.y, xy_res.x, CV_32FC3);

			tbb::parallel_for(
			    tbb::blocked_range2d<int>(0, xy_res.y, 0, xy_res.x), [&](const tbb::blocked_range2d<int>& range) {
				    for(int y = range.rows().begin(); y != range.rows().end(); ++y)
					    for(int x = range.cols().begin(); x != range.cols().end(); ++x) {
						    float* pixel = m.ptr<float>(y, x);

						    for(int c = 0; c < 3; ++c)
							    pixel[c] = bspline[c].sample(
							        std::array<float, 4>{{(float)x * xy_scale.x, (float)y * xy_scale.y, uv.x, uv.y}});
					    }
			    });

			seq(u - uv_res.x / 2, v - uv_res.y / 2) = std::move(m);
		});
	});

	data.set(a_sequence, seq);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_bspline, "bspline", std::array<possumwood::opencv::BSpline<4>, 3>(),
	                  possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_xyResolution, "xy_resolution", Imath::V2i(100, 100));
	meta.addAttribute(a_uvResolution, "uv_resolution", Imath::V2i(5, 5));
	meta.addAttribute(a_sequence, "sequence", possumwood::opencv::Sequence());

	meta.addInfluence(a_bspline, a_sequence);
	meta.addInfluence(a_xyResolution, a_sequence);
	meta.addInfluence(a_uvResolution, a_sequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/bspline_to_sequence", init);

}  // namespace
