#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_out;

enum Mode {
	kMinMax,
	kMax,
	kSum
};

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& in = data.get(a_in);

	if(!in.isValid())
		throw std::runtime_error("Input sequence does not have consistent size or type.");

	possumwood::opencv::Sequence out = in.clone();

	if(!in.empty()) {
		if(in.type() != CV_32FC1)
			throw std::runtime_error("Only 1-channel float images supported for the moment.");

		const int cols = in.cols();
		const int rows = in.rows();

		Mode mode = kMinMax;
		if(data.get(a_mode).value() == "Maximum")
			mode = kMax;
		else if(data.get(a_mode).value() == "Sum")
			mode = kSum;

		tbb::parallel_for(0, rows, [&](int r) {
			for(int c=0; c<cols; ++c) {
				auto it = out.begin();

				float max = (*it)->at<float>(r, c);
				float min = max;
				float sum = max;
				++it;

				while(it != out.end()) {
					const float& tmp = (*it)->at<float>(r, c);

					max = std::max(max, tmp);
					min = std::min(min, tmp);
					sum += tmp;

					++it;
				}

				if(mode == kMinMax)
					for(it = out.begin(); it != out.end(); ++it) {
						float& val = (*it)->at<float>(r, c);
						val = (val - min) / (max - min);
					}
				else if(mode == kMax)
					for(it = out.begin(); it != out.end(); ++it) {
						float& val = (*it)->at<float>(r, c);
						val /= max;
					}
				else if(mode == kSum)
					for(it = out.begin(); it != out.end(); ++it) {
						float& val = (*it)->at<float>(r, c);
						val /= sum;
					}
			}
		});
	}

	data.set(a_out, out);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in");
	meta.addAttribute(a_mode, "mode",
	                  possumwood::Enum({"Minimum-maximum", "Maximum", "Sum"}));
	meta.addAttribute(a_out, "out");

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_mode, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/pixel_normalize", init);

}
