#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<float> a_offset;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& input = data.get(a_inSequence);
	possumwood::opencv::Sequence result;

	tbb::task_group group;

	for(auto it = input.begin(); it != input.end(); ++it) {
		group.run([&data, it, &result]() {
			cv::Mat tmp = it->second;
			tmp += cv::Scalar::all(data.get(a_offset));
			cv::log(tmp, tmp);

			result[it->first] = std::move(tmp);
		});
	}

	group.wait();

	data.set(a_outSequence, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_seq", possumwood::opencv::Sequence());
	meta.addAttribute(a_offset, "offset", 1.0f);
	meta.addAttribute(a_outSequence, "out_seq", possumwood::opencv::Sequence());

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_offset, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/log", init);

}  // namespace
