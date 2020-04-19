#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_h, a_photoRender;
dependency_graph::InAttr<unsigned> a_searchWindow, a_blockSize;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat mat;

	const cv::Mat& in = *data.get(a_inFrame);

	if(in.rows > 0 && in.cols > 0) {
		if(in.type() == CV_8UC1 || in.type() == CV_32FC1)
			cv::fastNlMeansDenoising(*data.get(a_inFrame), mat, data.get(a_h), data.get(a_searchWindow), data.get(a_blockSize));

		else if(in.type() == CV_8UC3 || in.type() == CV_32FC3)
			cv::fastNlMeansDenoisingColored(*data.get(a_inFrame), mat, data.get(a_h), data.get(a_photoRender), data.get(a_searchWindow), data.get(a_blockSize));

		// else if(mat.type() == CV_32FC3)

		else
			throw std::runtime_error("Unsupported data type " + possumwood::opencv::type2str(mat.type()) + " - needs extending to support this type!");
	}

	data.set(a_outFrame, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_h, "h", 1.0f);
	meta.addAttribute(a_photoRender, "photo_render", 10.0f);
	meta.addAttribute(a_searchWindow, "search_window", 21u);
	meta.addAttribute(a_blockSize, "block_size", 7u);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_h, a_outFrame);
	meta.addInfluence(a_photoRender, a_outFrame);
	meta.addInfluence(a_searchWindow, a_outFrame);
	meta.addInfluence(a_blockSize, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/filter/denoise", init);

}
