#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

#include "maths/io/vec2.h"
#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<Imath::V2i> a_xRange, a_yRange;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& input = data.get(a_inSequence);

    const Imath::V2i xRange = data.get(a_xRange);
	const Imath::V2i yRange = data.get(a_yRange);

	possumwood::opencv::Sequence result;

	for(auto it = input.begin(); it != input.end(); ++it) {
        if(it->first.x >= xRange[0] && it->first.x <= xRange[1] && it->first.y >= yRange[0] && it->first.y <= yRange[1]) {
            cv::Mat m = it->second; // shallow copy, but as we are not modifying anything, that should be fine
            result[it->first] = std::move(m);
        }
	}

	data.set(a_outSequence, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_seq", possumwood::opencv::Sequence());
	meta.addAttribute(a_xRange, "range/x", Imath::V2i(-5,5));
	meta.addAttribute(a_yRange, "range/y", Imath::V2i(-5, 5));
	meta.addAttribute(a_outSequence, "out_seq", possumwood::opencv::Sequence());

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_xRange, a_outSequence);
	meta.addInfluence(a_yRange, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/subset", init);

}  // namespace
