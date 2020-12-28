#include <possumwood_sdk/node_implementation.h>

#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>

#include <maths/io/vec2.h>

#include "datatypes/bspline.h"

#include "bspline.inl"
#include "frame.h"

namespace {

dependency_graph::InAttr<std::array<possumwood::opencv::BSpline<4>, 3>> a_bspline;
dependency_graph::InAttr<Imath::V2i> a_resolution;
dependency_graph::InAttr<Imath::V2f> a_uv;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Imath::V2i& res = data.get(a_resolution);

	const std::array<possumwood::opencv::BSpline<4>, 3>& bspline = data.get(a_bspline);

	Imath::V2f uv = data.get(a_uv);
	uv = (uv + Imath::V2f(1.0f, 1.0f)) / 2.0f * Imath::V2f(bspline[0].size(2) - 1, bspline[0].size(3) - 1);

	Imath::V2f xy_scale =
	    Imath::V2f(bspline[0].size(0) - 1, bspline[0].size(1) - 1) / Imath::V2f(data.get(a_resolution));

	cv::Mat result = cv::Mat::zeros(res.y, res.x, CV_32FC3);

	tbb::parallel_for(tbb::blocked_range2d<int>(0, res.y, 0, res.x), [&](const tbb::blocked_range2d<int>& range) {
		for(int y = range.rows().begin(); y != range.rows().end(); ++y)
			for(int x = range.cols().begin(); x != range.cols().end(); ++x) {
				float* pixel = result.ptr<float>(y, x);

				for(int c = 0; c < 3; ++c)
					pixel[c] = bspline[c].sample(
					    std::array<float, 4>{{(float)x * xy_scale.x, (float)y * xy_scale.y, uv.x, uv.y}});
			}
	});

	data.set(a_frame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_bspline, "bspline", std::array<possumwood::opencv::BSpline<4>, 3>(),
	                  possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_resolution, "resolution", Imath::V2i(10, 10));
	meta.addAttribute(a_uv, "uv", Imath::V2f(0, 0));
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_bspline, a_frame);
	meta.addInfluence(a_resolution, a_frame);
	meta.addInfluence(a_uv, a_frame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/bspline_to_frame", init);

}  // namespace
