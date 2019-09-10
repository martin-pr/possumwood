#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <tbb/task_group.h>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame, a_inMask;
dependency_graph::InAttr<float> a_radius;
dependency_graph::InAttr<possumwood::Enum> a_method;
dependency_graph::InAttr<unsigned> a_mosaic;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int methodToEnum(const std::string& method) {
	if(method == "INPAINT_NS")
		return cv::INPAINT_NS;
	else if(method == "INPAINT_TELEA")
		return cv::INPAINT_TELEA;

	throw std::runtime_error("Unknown inpaint method " + method);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	if(data.get(a_mosaic) < 1u)
		throw std::runtime_error("Mosaic parameter should be 1 or above!");

	const cv::Mat& input = *data.get(a_inFrame);
	const cv::Mat& mask = *data.get(a_inMask);
	cv::Mat result(input.rows, input.cols, input.type());

	const unsigned mosaic = data.get(a_mosaic);
	const float radius = data.get(a_radius);
	const int method = methodToEnum(data.get(a_method).value());

	tbb::task_group tasks;

	for(unsigned a=0;a<mosaic; ++a) {
		for(unsigned b=0;b<mosaic; ++b) {
			cv::Rect2i roi;
			roi.y = (a * input.rows) / mosaic;
			roi.x = (b * input.cols) / mosaic;
			roi.height = ((a+1) * input.rows) / mosaic - roi.y;
			roi.width = ((b+1) * input.cols) / mosaic - roi.x;

			tasks.run([&input, &mask, &result, roi, &radius, &method]() {
				cv::Mat inTile(input, roi);
				cv::Mat inMask(mask, roi);
				cv::Mat outResult(result, roi);

				cv::inpaint(inTile, inMask, outResult, radius, method);
			});
		}
	}

	tasks.wait();

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inMask, "mask", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_radius, "radius", 5.0f);
	meta.addAttribute(a_method, "method", possumwood::Enum({"INPAINT_NS", "INPAINT_TELEA"}));
	meta.addAttribute(a_mosaic, "mosaic", 1u);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_inMask, a_outFrame);
	meta.addInfluence(a_radius, a_outFrame);
	meta.addInfluence(a_method, a_outFrame);
	meta.addInfluence(a_mosaic, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/inpaint", init);

}
