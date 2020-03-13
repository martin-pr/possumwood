#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& in = data.get(a_in);

	if(!in.isValid())
		throw std::runtime_error("Input sequence does not have consistent size or type.");

	possumwood::opencv::Sequence out = in;

	if(!in.empty()) {
		if(in.type() != CV_32FC1)
			throw std::runtime_error("Only 1-channel float images supported for the moment.");

		const int cols = in.cols();
		const int rows = in.rows();

		tbb::parallel_for(0, rows, [&](int r) {
			for(int c=0; c<cols; ++c) {
				auto it = out.begin();
				float max = (*it)->at<float>(r, c);
				float min = (*it)->at<float>(r, c);
				++it;

				while(it != out.end()) {
					max = std::max(max, (*it)->at<float>(r, c));
					min = std::min(min, (*it)->at<float>(r, c));

					++it;
				}

				for(it = out.begin(); it != out.end(); ++it) {
					float& val = (*it)->at<float>(r, c);
					val = (val - min) / (max - min);
				}
			}
		});
	}

	data.set(a_out, out);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in");
	meta.addAttribute(a_out, "out");

	meta.addInfluence(a_in, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/pixel_normalize", init);

}
