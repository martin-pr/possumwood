#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

struct Params {
	dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
	dependency_graph::InAttr<unsigned> a_iterations;
	dependency_graph::InAttr<possumwood::Enum> a_borderType;
	dependency_graph::InAttr<float> a_borderValue;
	dependency_graph::InAttr<possumwood::Enum> a_kernelShape;
	dependency_graph::InAttr<unsigned> a_kernelSize;
	dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;
};

int borderToEnum(const std::string& mode) {
	if(mode == "BORDER_CONSTANT")
		return cv::BORDER_CONSTANT;
	else if(mode == "BORDER_REPLICATE")
		return cv::BORDER_REPLICATE;
	else if(mode == "BORDER_REFLECT")
		return cv::BORDER_REFLECT;
	else if(mode == "BORDER_WRAP")
		return cv::BORDER_WRAP;
	else if(mode == "BORDER_REFLECT_101")
		return cv::BORDER_REFLECT_101;
	else if(mode == "BORDER_TRANSPARENT")
		return cv::BORDER_TRANSPARENT;
	else if(mode == "BORDER_REFLECT101")
		return cv::BORDER_REFLECT101;
	else if(mode == "BORDER_ISOLATED")
		return cv::BORDER_ISOLATED;

	throw std::runtime_error("Enum conversion error - unknown border mode " + mode);
}

int shapeToEnum(const std::string& mode) {
	if(mode == "MORPH_RECT")
		return cv::MORPH_RECT;
	else if(mode == "MORPH_ELLIPSE")
		return cv::MORPH_ELLIPSE;
	else if(mode == "MORPH_CROSS")
		return cv::MORPH_CROSS;

	throw std::runtime_error("Enum conversion error - unknown kernel shape mode " + mode);
}

template<typename FN>
dependency_graph::State compute(dependency_graph::Values& data, FN fn, Params& params) {
	cv::Mat result;

	const cv::Mat kernel = cv::getStructuringElement(
		shapeToEnum(data.get(params.a_kernelShape).value()),
		cv::Size(data.get(params.a_kernelSize), data.get(params.a_kernelSize))
	);

	fn(*data.get(params.a_inFrame), result, kernel, cv::Point(-1, -1), data.get(params.a_iterations), borderToEnum(data.get(params.a_borderType).value()), data.get(params.a_borderValue));

	data.set(params.a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

template<typename FN>
void init(possumwood::Metadata& meta, FN fn, Params& params) {
	meta.addAttribute(params.a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addAttribute(params.a_iterations, "iterations", 1u);

	meta.addAttribute(params.a_borderType, "border/type",
		possumwood::Enum({
			"BORDER_REPLICATE", "BORDER_REFLECT", "BORDER_WRAP", "BORDER_REFLECT_101",
			"BORDER_TRANSPARENT", "BORDER_REFLECT101", "BORDER_ISOLATED", "BORDER_CONSTANT"
		}));
	meta.addAttribute(params.a_borderValue, "border/value");
	meta.addAttribute(params.a_kernelShape, "kernel/shape",
		possumwood::Enum({
			"MORPH_RECT", "MORPH_ELLIPSE", "MORPH_CROSS"
		}));
	meta.addAttribute(params.a_kernelSize, "kernel/size", 3u);
	meta.addAttribute(params.a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(params.a_inFrame, params.a_outFrame);
	meta.addInfluence(params.a_iterations, params.a_outFrame);
	meta.addInfluence(params.a_borderType, params.a_outFrame);
	meta.addInfluence(params.a_borderValue, params.a_outFrame);
	meta.addInfluence(params.a_kernelShape, params.a_outFrame);
	meta.addInfluence(params.a_kernelSize, params.a_outFrame);

	meta.setCompute([fn, &params](dependency_graph::Values& data) {
		return compute(data, fn, params);
	});
}

static Params s_dilateParams;
possumwood::NodeImplementation s_implDilate("opencv/dilate", [&](possumwood::Metadata& meta) {
	init(meta, cv::dilate, s_dilateParams);
});

static Params s_erodeParams;
possumwood::NodeImplementation s_implErode("opencv/erode", [&](possumwood::Metadata& meta) {
	init(meta, cv::erode, s_erodeParams);
});

}
