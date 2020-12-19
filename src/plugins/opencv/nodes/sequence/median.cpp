#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/node_implementation.h>

#include <actions/traits.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<unsigned> a_kernelSize;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& input = data.get(a_inSequence);
	possumwood::opencv::Sequence result;

	tbb::task_group group;

	for(auto it = input.begin(); it != input.end(); ++it) {
		group.run([&data, it, &result]() {
			cv::Mat tmp = it->second;

			cv::medianBlur(tmp, tmp, data.get(a_kernelSize));

			result[it->first] = std::move(tmp);
		});
	}

	group.wait();

	data.set(a_outSequence, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_seq", possumwood::opencv::Sequence());
	meta.addAttribute(a_kernelSize, "kernel_size", 5u);
	meta.addAttribute(a_outSequence, "out_seq", possumwood::opencv::Sequence());

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_kernelSize, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/median", init);

}  // namespace
