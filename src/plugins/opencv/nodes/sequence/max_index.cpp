#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& in = data.get(a_in);

	cv::Mat out = cv::Mat::zeros(in.meta().rows, in.meta().cols, CV_8UC1);

	if(in.meta().type != CV_32FC1)
		throw std::runtime_error("Only 1-channel float images supported for the moment.");

	if(!in.empty() && !in.hasOneElement()) {
		const int cols = in.meta().cols;
		const int rows = in.meta().rows;

		tbb::parallel_for(0, rows, [&](int r) {
			for(int c = 0; c < cols; ++c) {
				auto it = in.begin();

				std::size_t max_id = 0;
				float max = it->second.at<float>(r, c);
				++it;

				std::size_t counter = 1;
				while(it != in.end()) {
					const float current = it->second.at<float>(r, c);
					if(max < current) {
						max = current;
						max_id = counter;
					}

					++it;
					++counter;
				}

				out.at<unsigned char>(r, c) = max_id;
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

}  // namespace
