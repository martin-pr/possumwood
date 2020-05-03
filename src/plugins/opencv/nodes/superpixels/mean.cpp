#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::opencv::Frame> a_superpixels;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

float getFloat(const cv::Mat& in, int row, int col, int channel) {
	switch(in.depth()) {
		case CV_8U: return in.ptr<uint8_t>(row, col)[channel];
		case CV_8S: return in.ptr<int8_t>(row, col)[channel];
		case CV_16U: return in.ptr<uint16_t>(row, col)[channel];
		case CV_16S: return in.ptr<int16_t>(row, col)[channel];
		case CV_32S: return in.ptr<int32_t>(row, col)[channel];
		case CV_32F: return in.ptr<float>(row, col)[channel];
		case CV_64F: return in.ptr<double>(row, col)[channel];
		default: throw std::runtime_error("Unknown number type in input.");
	}
}

void setFloat(cv::Mat& in, int row, int col, int channel, float val) {
	switch(in.depth()) {
		case CV_8U: in.ptr<uint8_t>(row, col)[channel] = val; break;
		case CV_8S: in.ptr<int8_t>(row, col)[channel] = val; break;
		case CV_16U: in.ptr<uint16_t>(row, col)[channel] = val; break;
		case CV_16S: in.ptr<int16_t>(row, col)[channel] = val; break;
		case CV_32S: in.ptr<int32_t>(row, col)[channel] = val; break;
		case CV_32F: in.ptr<float>(row, col)[channel] = val; break;
		case CV_64F: in.ptr<double>(row, col)[channel] = val; break;
		default: throw std::runtime_error("Unknown number type in input.");
	}
}

class Mean {
	public:
		Mean() : m_val(0.0f), m_norm(0.0f) {
		}

		Mean& operator += (float val) {
			m_val += val;
			m_norm += 1.0f;

			return *this;
		}

		float operator*() const {
			if(m_norm > 0.0f)
				return m_val / m_norm;
			return 0.0f;
		}

	private:
		float m_val, m_norm;
};

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

	// make the right sized accumulator and norm array
	std::vector<std::vector<Mean>> vals(in.channels(), std::vector<Mean>(maxIndex+1, Mean()));

	// and accumulate the values
	for(int row=0; row<in.rows; ++row)
		for(int col=0; col<in.cols; ++col) {
			const int32_t index = superpixels.at<int32_t>(row, col);

			for(int c=0;c<in.channels();++c)
				vals[c][index] += getFloat(in, row, col, c);
		}

	// assign the result
	for(int row=0; row<in.rows; ++row)
		for(int col=0; col<in.cols; ++col)
			for(int c=0;c<in.channels();++c) {
				const int32_t index = superpixels.at<int32_t>(row, col);
				setFloat(out, row, col, c, *vals[c][index]);
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
