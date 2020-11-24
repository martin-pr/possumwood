#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence1, a_outSequence2, a_outSequence3, a_outSequence4;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& in = data.get(a_inSequence);

	std::vector<possumwood::opencv::Sequence> result(4, possumwood::opencv::Sequence(in.size()));

	for(std::size_t a = 0; a < in.size(); ++a) {
		if(in(a).rows > 0 && in(a).cols > 0) {
			std::vector<cv::Mat> mats(4);
			cv::split(in(a), mats);

			for(std::size_t i = 0; i < 4; ++i)
				result[i](a) = std::move(mats[i]);
		}
	}

	data.set(a_outSequence1, possumwood::opencv::Sequence(result[0]));
	data.set(a_outSequence2, possumwood::opencv::Sequence(result[1]));
	data.set(a_outSequence3, possumwood::opencv::Sequence(result[2]));
	data.set(a_outSequence4, possumwood::opencv::Sequence(result[3]));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in");
	meta.addAttribute(a_outSequence1, "out_1");
	meta.addAttribute(a_outSequence2, "out_2");
	meta.addAttribute(a_outSequence3, "out_3");
	meta.addAttribute(a_outSequence4, "out_4");

	meta.addInfluence(a_inSequence, a_outSequence1);
	meta.addInfluence(a_inSequence, a_outSequence2);
	meta.addInfluence(a_inSequence, a_outSequence3);
	meta.addInfluence(a_inSequence, a_outSequence4);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/split", init);

}  // namespace
