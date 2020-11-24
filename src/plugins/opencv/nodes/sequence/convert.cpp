#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSeq;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<float> a_a, a_b;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSeq;

static const std::vector<std::pair<std::string, int>> s_modes{{"CV_8U", CV_8U}, {"CV_16U", CV_16U}, {"CV_32F", CV_32F}};

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& in = data.get(a_inSeq);

	possumwood::opencv::Sequence out(in.size());

	tbb::parallel_for(std::size_t(0), in.size(), [&](std::size_t id) {
		cv::Mat m;
		in(id).convertTo(m, data.get(a_mode).intValue(), data.get(a_a), data.get(a_b));

		out(id) = std::move(m);
	});

	data.set(a_outSeq, out);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSeq, "in_sequence");
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_modes.begin(), s_modes.end()));
	meta.addAttribute(a_a, "a", 1.0f);
	meta.addAttribute(a_b, "b", 0.0f);
	meta.addAttribute(a_outSeq, "out_sequence");

	meta.addInfluence(a_inSeq, a_outSeq);
	meta.addInfluence(a_mode, a_outSeq);
	meta.addInfluence(a_a, a_outSeq);
	meta.addInfluence(a_b, a_outSeq);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/convert", init);

}  // namespace
