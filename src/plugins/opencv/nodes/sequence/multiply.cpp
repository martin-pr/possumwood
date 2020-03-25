#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <tbb/parallel_for.h>

#include <actions/traits.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSeq;
dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSeq;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& in = data.get(a_inSeq).clone();
	const cv::Mat& m = *data.get(a_inFrame);

	possumwood::opencv::Sequence out(in.size());

	tbb::parallel_for(std::size_t(0), in.size(), [&](std::size_t id) {
		cv::multiply(*data.get(a_inSeq)[id], m, *out[id]);
	});

	data.set(a_outSeq, out);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSeq, "in");
	meta.addAttribute(a_inFrame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outSeq, "out");

	meta.addInfluence(a_inSeq, a_outSeq);
	meta.addInfluence(a_inFrame, a_outSeq);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/multiply", init);

}
