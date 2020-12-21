#include <possumwood_sdk/node_implementation.h>

#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>

#include <lightfields/dct.h>

#include <maths/io/vec2.h>

#include "datatypes/dct.h"
#include "frame.h"
#include "lightfields.h"

namespace {

dependency_graph::InAttr<lightfields::DCT> a_dct;
dependency_graph::InAttr<Imath::V2i> a_resolution;
dependency_graph::InAttr<Imath::V2f> a_uv;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Imath::V2i& res = data.get(a_resolution);
	const Imath::V2f& uv = data.get(a_uv);

	const lightfields::DCT& dct = data.get(a_dct);

	cv::Mat result = cv::Mat::zeros(res.y, res.x, CV_32FC3);

	tbb::parallel_for(tbb::blocked_range2d<int>(0, res.y, 0, res.x), [&](const tbb::blocked_range2d<int>& range) {
		for(int y = range.rows().begin(); y != range.rows().end(); ++y)
			for(int x = range.cols().begin(); x != range.cols().end(); ++x) {
				float* pixel = result.ptr<float>(y, x);

				const auto& arr = dct.get((float)x / (float)(res.x - 1), (float)y / (float)(res.y - 1), uv.x, uv.y);
				for(int c = 0; c < 3; ++c)
					pixel[c] = arr[c];
			}
	});

	data.set(a_frame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_dct, "dct", lightfields::DCT(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_resolution, "resolution", Imath::V2i(100, 100));
	meta.addAttribute(a_uv, "uv", Imath::V2f(0, 0));
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_dct, a_frame);
	meta.addInfluence(a_resolution, a_frame);
	meta.addInfluence(a_uv, a_frame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/dct_to_image", init);

}  // namespace
