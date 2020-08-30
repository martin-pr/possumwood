#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<unsigned> a_mod;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

template <typename T>
void process(cv::Mat& data, const T& val) {
	tbb::parallel_for(0, data.rows, [&](int row) {
		for(int col = 0; col < data.cols; ++col) {
			T* ptr = data.ptr<T>(row, col);
			for(int a = 0; a < data.channels(); ++a)
				ptr[a] = ptr[a] % val;
		}
	});
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat m = data.get(a_in)->clone();

	if(m.depth() == CV_8U)
		process<uint8_t>(m, data.get(a_mod));
	else if(m.depth() == CV_8S)
		process<int8_t>(m, data.get(a_mod));
	else if(m.depth() == CV_16U)
		process<uint16_t>(m, data.get(a_mod));
	else if(m.depth() == CV_16S)
		process<int16_t>(m, data.get(a_mod));
	else if(m.depth() == CV_32S)
		process<int32_t>(m, data.get(a_mod));
	else
		throw std::runtime_error("Only integer types supported for modulo operation.");

	data.set(a_out, possumwood::opencv::Frame(m));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mod, "modulo", 2u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_mod, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/maths/mod", init);

}  // namespace
