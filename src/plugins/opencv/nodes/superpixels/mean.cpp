#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::opencv::Frame> a_superpixels;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

float getFloat(const cv::Mat& in, int row, int col, int channel) {
	if(in.depth() == CV_32F)
		return in.ptr<float>(row, col)[channel];
	return in.ptr<double>(row, col)[channel];
}

long getLong(const cv::Mat& in, int row, int col, int channel) {
	if(in.depth() == CV_8U)
		return in.ptr<uint8_t>(row, col)[channel];
	if(in.depth() == CV_8S)
		return in.ptr<int8_t>(row, col)[channel];
	if(in.depth() == CV_16U)
		return in.ptr<uint16_t>(row, col)[channel];
	if(in.depth() == CV_16S)
		return in.ptr<int16_t>(row, col)[channel];
	return in.ptr<int32_t>(row, col)[channel];
}

void setFloat(cv::Mat& in, int row, int col, int channel, float val) {
	if(in.depth() == CV_32F)
		in.ptr<float>(row, col)[channel] = val;
	else
		in.ptr<double>(row, col)[channel] = val;
}

void setLong(cv::Mat& in, int row, int col, int channel, long val) {
	if(in.depth() == CV_8U)
		in.ptr<uint8_t>(row, col)[channel] = val;
	else if(in.depth() == CV_8S)
		in.ptr<int8_t>(row, col)[channel] = val;
	else if(in.depth() == CV_16U)
		in.ptr<uint16_t>(row, col)[channel] = val;
	else if(in.depth() == CV_16S)
		in.ptr<int16_t>(row, col)[channel] = val;
	else
		in.ptr<int32_t>(row, col)[channel] = val;
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_in);
	const cv::Mat& superpixels = *data.get(a_superpixels);

	if(in.rows != superpixels.rows || in.cols != superpixels.cols)
		throw std::runtime_error("Input and superpixel size have to match.");

	if(superpixels.type() != CV_32SC1)
		throw std::runtime_error("Only CV_32SC1 type supported on the superpixels input!");

	// first of all, get the maximum index of the superpixels
	int32_t maxIndex = 0;
	for(int row=0; row<superpixels.rows; ++row)
		for(int col=0; col<superpixels.cols; ++col)
			maxIndex = std::max(maxIndex, superpixels.at<int32_t>(row, col));

	cv::Mat out = cv::Mat::zeros(in.rows, in.cols, in.type());

	if(in.depth() == CV_32F || in.depth() == CV_64F) {
		// make the right sized accumulator and norm array
		std::vector<std::vector<float>> vals(in.channels(), std::vector<float>(maxIndex+1, 0.0f));
		std::vector<long> norm(maxIndex+1, 0);

		// and accumulate the values
		for(int row=0; row<in.rows; ++row)
			for(int col=0; col<in.cols; ++col) {
				const int32_t index = superpixels.at<int32_t>(row, col);

				for(int c=0;c<in.channels();++c)
					vals[c][index] += getFloat(in, row, col, c);

				norm[index]++;
			}

		// assign the result
		for(int row=0; row<in.rows; ++row)
			for(int col=0; col<in.cols; ++col)
				for(int c=0;c<in.channels();++c) {
					const int32_t index = superpixels.at<int32_t>(row, col);
					setFloat(out, row, col, c, vals[c][index] / (float)norm[index]);
				}
	}

	else {
		// make the right sized accumulator and norm array
		std::vector<std::vector<long>> vals(in.channels(), std::vector<long>(maxIndex+1, 0.0f));
		std::vector<long> norm(maxIndex+1, 0);

		// and accumulate the values
		for(int row=0; row<in.rows; ++row)
			for(int col=0; col<in.cols; ++col) {
				const int32_t index = superpixels.at<int32_t>(row, col);

				for(int c=0;c<in.channels();++c)
					vals[c][index] += getLong(in, row, col, c);

				norm[index]++;
			}

		// assign the result
		for(int row=0; row<in.rows; ++row)
			for(int col=0; col<in.cols; ++col)
				for(int c=0;c<in.channels();++c) {
					const int32_t index = superpixels.at<int32_t>(row, col);
					setLong(out, row, col, c, vals[c][index] / norm[index]);
				}
	}

	data.set(a_out, possumwood::opencv::Frame(out));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_superpixels, "superpixels", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "out", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_superpixels, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/superpixels/mean", init);

}
