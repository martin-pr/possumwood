#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>
#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

#include "maths/io/vec2.h"
#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::InAttr<float> a_offset;
dependency_graph::InAttr<bool> a_circularFilter;
dependency_graph::InAttr<float> a_circularThreshold;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out, a_mask;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& input = data.get(a_in);
	if(input.meta().type != CV_32FC3)
		throw std::runtime_error("Only 32-bit 3 channel float format supported on input, " +
		                         possumwood::opencv::type2str(input.meta().type) + " found instead!");

	const int width = data.get(a_size)[0];
	const int height = data.get(a_size)[1];

	float mosaic_size = std::max(input.max().x - input.min().x + 1, input.max().y - input.min().y + 1);

	auto mci = (input.max() - input.min());
	const Imath::V2f mosaic_center = Imath::V2f(mci.x, mci.y) / 2.0f;

	const float offset = data.get(a_offset);

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_16UC3);

	const bool circular_filter = data.get(a_circularFilter);
	const float circular_threshold = powf(mosaic_size * data.get(a_circularThreshold) / 2, 2);

	tbb::task_group group;

	for(auto it = input.begin(); it != input.end(); ++it) {
		if(!circular_filter || (Imath::V2f(it->first) - mosaic_center).length2() <= circular_threshold) {
			group.run([it, &mat, &norm, &offset, &mosaic_center]() {
				const Imath::V2f offs = (Imath::V2f(it->first.x, it->first.y) - mosaic_center) * (float)offset;
				for(int y = 0; y < it->second.rows; ++y)
					for(int x = 0; x < it->second.cols; ++x) {
						const Imath::V2f posf((float)x / (float)it->second.cols * (float)mat.cols + offs.x,
						                      (float)y / (float)it->second.rows * (float)mat.rows + offs.y);

						const Imath::V2i pos(round(posf.x), round(posf.y));

						if(pos.x >= 0 && pos.x < mat.cols && pos.y >= 0 && pos.y < mat.rows) {
							float* color = mat.ptr<float>(pos.y, pos.x);
							int16_t* n = norm.ptr<int16_t>(pos.y, pos.x);

							const float* value = it->second.ptr<float>(y, x);

							for(int a = 0; a < 3; ++a) {
								color[a] += value[a];
								n[a] += 1;
							}
						}
					}
			});
		}
	}

	group.wait();

	tbb::parallel_for(0, mat.rows, [&](int y) {
		for(int x = 0; x < mat.cols; ++x)
			for(int a = 0; a < 3; ++a)
				if(norm.ptr<int16_t>(y, x)[a] > 0)
					mat.ptr<float>(y, x)[a] /= (float)norm.ptr<int16_t>(y, x)[a];
	});

	data.set(a_out, possumwood::opencv::Frame(mat));
	data.set(a_mask, possumwood::opencv::Frame(norm));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "sequence");
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(1000, 1000));
	meta.addAttribute(a_offset, "offset", 3.0f);
	meta.addAttribute(a_out, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mask, "mask", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_circularFilter, "filter/circular", true);
	meta.addAttribute(a_circularThreshold, "filter/threshold", 1.0f);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_size, a_out);
	meta.addInfluence(a_offset, a_out);
	meta.addInfluence(a_circularFilter, a_out);
	meta.addInfluence(a_circularThreshold, a_out);

	meta.addInfluence(a_in, a_mask);
	meta.addInfluence(a_size, a_mask);
	meta.addInfluence(a_offset, a_mask);
	meta.addInfluence(a_circularFilter, a_mask);
	meta.addInfluence(a_circularThreshold, a_mask);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/mosaic_superres", init);

}  // namespace
