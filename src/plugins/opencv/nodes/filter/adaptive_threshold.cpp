#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_const, a_maxVal;
dependency_graph::InAttr<possumwood::Enum> a_method, a_type;
dependency_graph::InAttr<unsigned> a_blockSize;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int methodToEnum(const std::string& method) {
	if(method == "ADAPTIVE_THRESH_MEAN_C")
		return cv::ADAPTIVE_THRESH_MEAN_C;
	else if(method == "ADAPTIVE_THRESH_GAUSSIAN_C")
		return cv::ADAPTIVE_THRESH_GAUSSIAN_C;

	throw std::runtime_error("Unknown adaptive method " + method);
}

int typeToEnum(const std::string& type) {
	if(type == "THRESH_BINARY")
		return cv::THRESH_BINARY;
	else if(type == "THRESH_BINARY_INV")
		return cv::THRESH_BINARY_INV;
	// else if(type == "THRESH_TRUNC")
	// 	return cv::THRESH_TRUNC;
	// else if(type == "THRESH_TOZERO")
	// 	return cv::THRESH_TOZERO;
	// else if(type == "THRESH_TOZERO_INV")
	// 	return cv::THRESH_TOZERO_INV;
	// else if(type == "THRESH_MASK")
	// 	return cv::THRESH_MASK;

	throw std::runtime_error("Unknown conversion type " + type);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = (*data.get(a_inFrame)).clone();

	int method = methodToEnum(data.get(a_method).value());
	int type = typeToEnum(data.get(a_type).value());

	cv::adaptiveThreshold(*data.get(a_inFrame), result, data.get(a_maxVal), method, type, data.get(a_blockSize), data.get(a_const));

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_const, "const", 127.0f);
	meta.addAttribute(a_maxVal, "max_val", 255.0f);
	meta.addAttribute(a_method, "adaptive_method", possumwood::Enum({"ADAPTIVE_THRESH_MEAN_C", "ADAPTIVE_THRESH_GAUSSIAN_C"}));
	meta.addAttribute(a_type, "threshold_type", possumwood::Enum({"THRESH_BINARY", "THRESH_BINARY_INV" /*, "THRESH_TRUNC", "THRESH_TOZERO", "THRESH_TOZERO_INV", "THRESH_MASK" */ }));
	meta.addAttribute(a_blockSize, "block_size", 21u);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_const, a_outFrame);
	meta.addInfluence(a_maxVal, a_outFrame);
	meta.addInfluence(a_method, a_outFrame);
	meta.addInfluence(a_type, a_outFrame);
	meta.addInfluence(a_blockSize, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/filter/adaptive_threshold", init);

}
