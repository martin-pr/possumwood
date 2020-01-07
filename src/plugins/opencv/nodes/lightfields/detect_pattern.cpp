#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<unsigned> a_border;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& input = *data.get(a_in);
	if(input.type() != CV_8UC1)
		throw std::runtime_error("Only CV_8UC1 type allowed on input");

	const std::size_t border = data.get(a_border);
	if(border < 1)
		throw std::runtime_error("Border value needs to be non-zero");
	if(std::size_t(input.rows) < 2*border+1 || std::size_t(input.cols) < 2*border+1)
		throw std::runtime_error("Border value too large for the size of the input image");

	cv::Mat mat(input.size(), CV_8UC1, cv::Scalar(0));

	// // create the subdiv object - this object is the thing that performs delaunay triangulation
	cv::Subdiv2D subdiv(cv::Rect(0, 0, input.cols, input.rows));

	// feed all the local minima into it, except the ones too close to the border
	for(std::size_t y=1; y<std::size_t(input.rows) - 1; ++y)
		for(std::size_t x=1; x<std::size_t(input.cols) - 1; ++x) {
			const unsigned char current = input.at<unsigned char>(y, x);
			bool isMaximum = true;
			for(std::size_t a=0;a<9;++a)
				if(a != 4 && input.at<unsigned char>(y + a/3 - 1, x + a%3 - 1) >= current)
					isMaximum = false;

			if(isMaximum) {
				mat.at<unsigned char>(y, x) = 255;
				subdiv.insert(cv::Point2f(x, y));
			}
		}

	// draw all edges from the Delaunay
	std::vector<cv::Vec4f> edgeList;
	subdiv.getEdgeList(edgeList);

	for(auto eit = edgeList.begin(); eit != edgeList.end();) {
		auto& e = *eit;

		if(e[0] > border && e[0] < input.cols - border && e[1] > border && e[1] < input.rows - border &&
			e[2] > border && e[2] < input.cols - border && e[3] > border && e[3] < input.rows - border) {

			cv::line(mat, cv::Point2f(e[0], e[1]), cv::Point2f(e[2], e[3]), cv::Scalar(128));

			++eit;
		}
		else
			eit = edgeList.erase(eit);
	}

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_border, "border", 20u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_border, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/detect_pattern", init);

}
