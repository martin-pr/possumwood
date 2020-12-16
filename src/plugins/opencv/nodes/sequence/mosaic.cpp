#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <tbb/task_group.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

enum Modes { kHorizontalID, kVerticalID };

dependency_graph::State compute(dependency_graph::Values& data) {
	const auto& sequence = data.get(a_inSequence);

	// result
	cv::Mat result =
	    cv::Mat::zeros(sequence.meta().rows * (sequence.max().y - sequence.min().y + 1),
	                   sequence.meta().cols * (sequence.max().x - sequence.min().x + 1), sequence.meta().type);

	tbb::task_group group;

	for(auto it = sequence.begin(); it != sequence.end(); ++it) {
		group.run([it, &result, &sequence]() {
			const int x = it->first.x - sequence.min().x;
			const int y = it->first.y - sequence.min().y;

			cv::Rect rect(x * sequence.meta().cols, y * sequence.meta().rows, sequence.meta().cols,
			              sequence.meta().rows);
			it->second.copyTo(result(rect));
		});
	}

	group.wait();

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "seq", possumwood::opencv::Sequence());
	meta.addAttribute(a_outFrame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inSequence, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/mosaic", init);

}  // namespace
