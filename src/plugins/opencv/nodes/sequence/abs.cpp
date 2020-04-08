#include <possumwood_sdk/node_implementation.h>
#include <opencv2/opencv.hpp>
#include <tbb/parallel_for.h>

#include <actions/traits.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::opencv::Sequence result = data.get(a_inSequence).clone();

	tbb::parallel_for(std::size_t(0), result.size(), [&](std::size_t i) {
		*result[i] = cv::abs(*result[i]);
	});

	data.set(a_outSequence, possumwood::opencv::Sequence(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_seq", possumwood::opencv::Sequence());
	meta.addAttribute(a_outSequence, "out_seq", possumwood::opencv::Sequence());

	meta.addInfluence(a_inSequence, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/abs", init);

}
