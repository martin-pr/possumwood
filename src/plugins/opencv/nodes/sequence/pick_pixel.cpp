#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_index;
dependency_graph::InAttr<possumwood::opencv::Sequence> a_seq;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

void copyPixel(int row, int col, const cv::Mat& src, cv::Mat& dest) {
	assert(row >= 0 && row < src.rows && col >= 0 && col < src.cols);
	assert(row >= 0 && row < dest.rows && col >= 0 && col < dest.cols);
	assert(src.type() == dest.type());

	for(int c = 0; c < src.channels(); ++c)
		switch(src.depth()) {
			case CV_8U:
				dest.ptr<uint8_t>(row, col)[c] = src.ptr<uint8_t>(row, col)[c];
				break;
			case CV_8S:
				dest.ptr<int8_t>(row, col)[c] = src.ptr<int8_t>(row, col)[c];
				break;
			case CV_16U:
				dest.ptr<uint16_t>(row, col)[c] = src.ptr<uint16_t>(row, col)[c];
				break;
			case CV_16S:
				dest.ptr<int16_t>(row, col)[c] = src.ptr<int16_t>(row, col)[c];
				break;
			case CV_32S:
				dest.ptr<int32_t>(row, col)[c] = src.ptr<int32_t>(row, col)[c];
				break;
			case CV_32F:
				dest.ptr<float>(row, col)[c] = src.ptr<float>(row, col)[c];
				break;
			case CV_64F:
				dest.ptr<double>(row, col)[c] = src.ptr<double>(row, col)[c];
				break;
			default:
				throw std::runtime_error("Unknown type for pixel copy.");
		}
};

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& seq = data.get(a_seq);
	if(!seq.isValid())
		throw std::runtime_error("Input sequence does not have consistent size or type.");

	const cv::Mat& index = *data.get(a_index);

	if(index.rows != seq.rows() || index.cols != seq.cols())
		throw std::runtime_error("Index and sequence need to have the same size!");
	if(index.type() != CV_8UC1)
		throw std::runtime_error("Only CV_8UC1 type supported as index for the moment.");

	cv::Mat out = cv::Mat::zeros(index.rows, index.cols, seq.type());

	tbb::parallel_for(0, index.rows, [&](int r) {
		for(int c = 0; c < index.cols; ++c) {
			const unsigned char i = index.at<unsigned char>(r, c);

			if(i >= seq.size())
				throw std::runtime_error("Index out of range of the input sequence.");

			copyPixel(r, c, *seq[i], out);
		}
	});

	data.set(a_out, possumwood::opencv::Frame(out));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_seq, "sequence");
	meta.addAttribute(a_index, "index", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "out", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_seq, a_out);
	meta.addInfluence(a_index, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/pick_pixel", init);

}  // namespace
