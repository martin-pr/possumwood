#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <tbb/task_group.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<float> a_offset;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

enum Modes { kInt, kFloat };

static const std::vector<std::pair<std::string, int>> s_modes{{"Nearest pixel", kInt}, {"Subpixel", kFloat}};

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& seq = data.get(a_inSequence);
	possumwood::opencv::Sequence result;

	if(!seq.empty()) {
		const float offset = data.get(a_offset);

		Imath::V2f start(0, 0);
		if(offset < 0.0f)
			start = -offset * Imath::V2f(seq.max() - seq.min());

		tbb::task_group group;

		for(auto it = seq.begin(); it != seq.end(); ++it) {
			group.run([&start, it, &offset, &seq, &result, &data] {
				const Imath::V2f current = start + Imath::V2f(it->first * Imath::V2i(offset, offset));

				cv::Mat f = it->second.clone();

				if(data.get(a_mode).intValue() == kInt) {
					f = f(cv::Rect(std::round(current.x), std::round(current.y),
					               f.cols - std::ceil(offset * float(seq.max().x - seq.min().x)),
					               f.rows - std::ceil(offset * float(seq.max().y - seq.max().y))));
				}
				else {
					const float width = (float)f.cols - std::ceil(offset * float(seq.max().x - seq.min().x));
					const float height = (float)f.rows - std::ceil(offset * float(seq.max().y - seq.min().y));

					cv::getRectSubPix(f, cv::Size(int(width), int(height)),
					                  cv::Point2f(current.x + width / 2.0f, current.y + height / 2.0f), f);
				}

				result[it->first] = std::move(f);
			});
		}

		group.wait();
	}

	data.set(a_outSequence, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_sequence");
	meta.addAttribute(a_offset, "offset", 1.0f);
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_modes.begin(), s_modes.end()));
	meta.addAttribute(a_outSequence, "out_sequence");

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_offset, a_outSequence);
	meta.addInfluence(a_mode, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/offset", init);

}  // namespace
