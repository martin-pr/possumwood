#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<float> a_offset;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

enum Modes { kInt, kFloat };

static const std::vector<std::pair<std::string, int>> s_modes{{"Nearest pixel", kInt}, {"Subpixel", kFloat}};

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::opencv::Sequence seq = data.get(a_inSequence).clone();

	if(!seq.empty()) {
		const float offset = data.get(a_offset);

		float start = 0.0f;
		if(offset < 0.0f)
			start = -offset * float(seq.size() - 1);

		for(std::size_t i = 0; i < seq.size(); ++i) {
			const float current = start + i * offset;
			cv::Mat& f = *seq[i];

			if(data.get(a_mode).intValue() == kInt) {
				f = f(cv::Rect(std::round(current), 0, f.cols - std::ceil(offset * float(seq.size() - 1)), f.rows));
			}
			else {
				const float width = (float)f.cols - std::ceil(offset * float(seq.size() - 1));

				cv::getRectSubPix(f, cv::Size(int(width), f.rows),
				                  cv::Point2f(current + width / 2.0f, (float)f.rows / 2.0f), f);
			}
		}
	}

	data.set(a_outSequence, seq);

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
