#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_threshold, a_maxVal;
dependency_graph::InAttr<possumwood::Enum> a_type;
dependency_graph::InAttr<bool> a_otsu, a_triangle;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int modeToEnum(const std::string& mode) {
	if(mode == "THRESH_BINARY")
		return cv::THRESH_BINARY;
	else if(mode == "THRESH_BINARY_INV")
		return cv::THRESH_BINARY_INV;
	else if(mode == "THRESH_TRUNC")
		return cv::THRESH_TRUNC;
	else if(mode == "THRESH_TOZERO")
		return cv::THRESH_TOZERO;
	else if(mode == "THRESH_TOZERO_INV")
		return cv::THRESH_TOZERO_INV;
	else if(mode == "THRESH_MASK")
		return cv::THRESH_MASK;

	throw std::runtime_error("Unknown conversion mode " + mode);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = (*data.get(a_inFrame)).clone();

	int mode = modeToEnum(data.get(a_type).value());
	if(data.get(a_otsu))
		mode |= cv::THRESH_OTSU;
	if(data.get(a_triangle))
		mode |= cv::THRESH_TRIANGLE;

	cv::threshold(*data.get(a_inFrame), result, data.get(a_threshold), data.get(a_maxVal), mode);

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_threshold, "threshold", 127.0f);
	meta.addAttribute(a_maxVal, "max_val", 255.0f);
	meta.addAttribute(a_type, "type", possumwood::Enum({"THRESH_BINARY", "THRESH_BINARY_INV", "THRESH_TRUNC", "THRESH_TOZERO", "THRESH_TOZERO_INV", "THRESH_MASK"}));
	meta.addAttribute(a_otsu, "use_otsu", false);
	meta.addAttribute(a_triangle, "use_triangle", false);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_threshold, a_outFrame);
	meta.addInfluence(a_maxVal, a_outFrame);
	meta.addInfluence(a_type, a_outFrame);
	meta.addInfluence(a_otsu, a_outFrame);
	meta.addInfluence(a_triangle, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/filter/threshold", init);

}
