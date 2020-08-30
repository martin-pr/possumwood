#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

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

	if(mask.rows != input.rows || mask.cols != input.cols)
		throw std::runtime_error("Input and mask sizes have to match!");

	if(mask.channels() > 1 && mask.channels() != input.channels())
		throw std::runtime_error("The number of mask channels should be either 1 or the same as input image channels.");

	const unsigned mosaic = data.get(a_mosaic);
	const float radius = data.get(a_radius);
	const int method = methodToEnum(data.get(a_method).value());

	cv::Mat result;

	tbb::task_group tasks;

	// single mask mode - use all channels of the input image for inpainting
	if(mask.channels() == 1) {
		result = cv::Mat(input.rows, input.cols, input.type());

		for(unsigned a = 0; a < mosaic; ++a) {
			for(unsigned b = 0; b < mosaic; ++b) {
				cv::Rect2i roi;
				roi.y = (a * input.rows) / mosaic;
				roi.x = (b * input.cols) / mosaic;
				roi.height = ((a + 1) * input.rows) / mosaic - roi.y;
				roi.width = ((b + 1) * input.cols) / mosaic - roi.x;

				tasks.run([&input, &mask, &result, roi, &radius, &method]() {
					cv::Mat inTile(input, roi);
					cv::Mat inMask(mask, roi);

					cv::Mat outResult(result, roi);

					cv::inpaint(inTile, inMask, outResult, radius, method);
				});
			}
		}

		tasks.wait();
	}

	// per-channel mode - use each channel of the mask for separate inpaining of each channel of the input
	else {
		// split the inputs and masks per channel
		std::vector<cv::Mat> inputs, masks;
		cv::split(input, inputs);
		cv::split(mask, masks);

		// initialise output as separate images (can't use resize because cv::Mat copy shares data and is not const
		// correct)
		std::vector<cv::Mat> results;
		for(std::size_t i = 0; i < inputs.size(); ++i)
			results.push_back(cv::Mat(input.rows, input.cols, input.depth()));

		// spin a task per tile and per channel
		for(unsigned a = 0; a < mosaic; ++a) {
			for(unsigned b = 0; b < mosaic; ++b) {
				cv::Rect2i roi;
				roi.y = (a * input.rows) / mosaic;
				roi.x = (b * input.cols) / mosaic;
				roi.height = ((a + 1) * input.rows) / mosaic - roi.y;
				roi.width = ((b + 1) * input.cols) / mosaic - roi.x;

				for(std::size_t c = 0; c < inputs.size(); ++c) {
					tasks.run([&inputs, &masks, &results, roi, &radius, &method, c]() {
						cv::Mat inTile(inputs[c], roi);

						cv::Mat inMask;
						if(masks.size() == 1)
							inMask = cv::Mat(masks[0], roi);
						else
							inMask = cv::Mat(masks[c], roi);

						cv::Mat outResult(results[c], roi);

						cv::inpaint(inTile, inMask, outResult, radius, method);
					});
				}
			}
		}

		tasks.wait();

		// and assemble the result
		cv::merge(results, result);
	}

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

}  // namespace
