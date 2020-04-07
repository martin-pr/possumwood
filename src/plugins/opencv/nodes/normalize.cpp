#include <possumwood_sdk/node_implementation.h>

#include <actions/traits.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "tools.h"
#include "frame.h"

namespace {

template<typename T>
struct Traits {
	static constexpr long norm() { return std::numeric_limits<T>::max(); };
	typedef long accumulator;
};

template<>
struct Traits<float> {
	static constexpr float norm() { return 1.0f; };
	typedef float accumulator;
};

template<typename T>
struct MinMax {
	static cv::Mat eval(const cv::Mat& input) {
		// make sure we don't overwrite the data
		cv::Mat m = input.clone();

		T min = m.at<T>(0,0);
		T max = m.at<T>(0,0);

		for(int y=0;y<m.rows;++y)
			for(int x=0;x<m.cols;++x) {
				const T current = m.at<T>(y, x);
				min = std::min(min, current);
				max = std::max(max, current);
			}

		if(max == min)
			throw std::runtime_error("Cannot normalize the result - no data on input (min == max, leads to division by zero)");

		for(int y=0;y<m.rows;++y)
			for(int x=0;x<m.cols;++x) {
				T& current = m.at<T>(y, x);
				current = (typename Traits<T>::accumulator(current - min) * Traits<T>::norm()) / typename Traits<T>::accumulator(max - min);
			}

		return m;
	}
};

template<typename T>
struct Max {
	static cv::Mat eval(const cv::Mat& input) {
		// make sure we don't overwrite the data
		cv::Mat m = input.clone();

		T max = m.at<T>(0,0);

		for(int y=0;y<m.rows;++y)
			for(int x=0;x<m.cols;++x) {
				const T current = m.at<T>(y, x);
				max = std::max(max, current);
			}

		if(max == 0)
			throw std::runtime_error("Cannot normalize the result - no data on input (max == 0, leads to division by zero)");

		for(int y=0;y<m.rows;++y)
			for(int x=0;x<m.cols;++x) {
				T& current = m.at<T>(y, x);
				current = (typename Traits<T>::accumulator(current) * Traits<T>::norm()) / typename Traits<T>::accumulator(max);
			}

		return m;
	}
};

template<template<class> typename FN>
cv::Mat process(const cv::Mat& in) {
	if(in.type() == CV_8UC1)
		return FN<uint8_t>::eval(in);
	else if(in.type() == CV_8SC1)
		return FN<int8_t>::eval(in);
	else if(in.type() == CV_16UC1)
		return FN<uint16_t>::eval(in);
	else if(in.type() == CV_16SC1)
		return FN<int16_t>::eval(in);
	else if(in.type() == CV_32FC1)
		return FN<float>::eval(in);

	throw std::runtime_error("Unsupported matrix type");
}

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_in);

	if(in.channels() > 1)
		throw std::runtime_error("Only single-channel normalization supported at the moment.");

	cv::Mat m;

	if(in.rows > 0 && in.cols > 0) {
		if(data.get(a_mode).value() == "Min-max")
			m = process<MinMax>(in);
		else if(data.get(a_mode).value() == "Max")
			m = process<Max>(in);
		else
			throw std::runtime_error("Unknown mode selected");
	}

	data.set(a_out, possumwood::opencv::Frame(m));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"Min-max", "Max"}));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_mode, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/normalize", init);

}
