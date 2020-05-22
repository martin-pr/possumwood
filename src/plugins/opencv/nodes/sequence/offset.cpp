#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<int> a_offset;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::opencv::Sequence seq = data.get(a_inSequence).clone();

	if(data.get(a_offset) > 0) {
		int offs = 0;
		for(auto& f : seq) {
			*f = (*f)(cv::Rect(offs, 0, f->cols - data.get(a_offset)*seq.size(), f->rows)).clone();

			offs += data.get(a_offset);
		}
	}
	else {
		int offs = -data.get(a_offset)*(int(seq.size())-1);
		for(auto& f : seq) {
			*f = (*f)(cv::Rect(offs, 0, f->cols - std::abs(data.get(a_offset)*int(seq.size())), f->rows)).clone();

			offs += data.get(a_offset);
		}
	}

	data.set(a_outSequence, seq);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_sequence");
	meta.addAttribute(a_offset, "offset", 1);
	meta.addAttribute(a_outSequence, "out_sequence");

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_offset, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/offset", init);

}
