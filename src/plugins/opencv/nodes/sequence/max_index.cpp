#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& in = data.get(a_in);

	if(!in.isValid())
		throw std::runtime_error("Input sequence does not have consistent size or type.");

	cv::Mat out = cv::Mat::zeros(in.rows(), in.cols(), CV_8UC1);

	if(in.size() > 1) {
		if(in.type() != CV_32FC1)
			throw std::runtime_error("Only 1-channel float images supported for the moment.");

		const int cols = in.cols();
		const int rows = in.rows();

		tbb::parallel_for(0, rows, [&](int r) {
			for(int c=0; c<cols; ++c) {
				auto it = in.begin();

				std::size_t max_id = 0;
				float max = (*it)->at<float>(r, c);
				++it;

				while(it != in.end()) {
					const float current = (*it)->at<float>(r, c);
					if(max < current) {
						max = current;
						max_id = it - in.begin();
					}

					++it;
				}

				out.at<unsigned char>(r, c) = (unsigned char)((max_id * 255) / (in.size() - 1));
			}
		});
	}

	else
		throw std::runtime_error("At least two frames in the input sequence required!");

	data.set(a_out, possumwood::opencv::Frame(out));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in");
	meta.addAttribute(a_out, "out", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/max_index", init);

}
