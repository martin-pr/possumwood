#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "lightfield_pattern.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::opencv::LightfieldPattern> a_pattern;
dependency_graph::InAttr<unsigned> a_width, a_height;
dependency_graph::InAttr<float> a_uvScale, a_sigma2;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::LightfieldPattern& pattern = data.get(a_pattern);

	if((*data.get(a_in)).type() != CV_32FC1)
		throw std::runtime_error("Only 16-bit single-float format supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	const cv::Mat& input = *data.get(a_in);

	const unsigned width = data.get(a_width);
	const unsigned height = data.get(a_height);

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_32FC3);

	const float uvScale = data.get(a_uvScale);
	const float sigma2 = data.get(a_sigma2) * (float)width / (float)input.cols;

	for(int y=0;y<input.rows;++y) {
		std::cout << "\r" << (float)(y) / (input.rows) * 100.0f << std::flush;

		for(int x=0;x<input.cols;++x) {
			const cv::Vec4f coords = pattern.sample(cv::Vec2i(x, y));

			const cv::Vec2f pos(coords[0] + coords[2]*uvScale, coords[1] + coords[3]*uvScale);

			const double uv_magnitude_2 = coords[2]*coords[2] + coords[3]*coords[3];

			if(uv_magnitude_2 <= 1.0 && pos[0] >= 0.0f && pos[1] >= 0.0f && pos[0] < (float)input.cols && pos[1] < (float)input.rows) {
				const float value = input.at<float>(y, x);

				const float target_x = pos[0]/input.cols * (float)width;
				const float target_y = pos[1]/input.rows * (float)height;

				int xFrom = std::max((int)floor(target_x - 3.0f*sigma2), 0);
				int xTo = std::min((int)floor(target_x + 3.0f*sigma2 + 1.0f), (int)width);
				int yFrom = std::max((int)floor(target_y - 3.0f*sigma2), 0);
				int yTo = std::min((int)floor(target_y + 3.0f*sigma2 + 1.0f), (int)height);

				for(int yt=yFrom; yt<yTo; ++yt) {

					for(int xt=xFrom; xt<xTo; ++xt) {
						const float dist2 = std::pow((float)xt - target_x, 2) + std::pow((float)yt - target_y, 2);
						const float gauss = std::exp(-dist2/(2.0f*sigma2));

						float* color = mat.ptr<float>(yt, xt);
						float* n = norm.ptr<float>(yt, xt);

						// hardcoded bayer pattern - blue is already correct, red and green require offsets
						color += (x%2) + (y%2);
						n += (x%2) + (y%2);

						*color += value * gauss;
						*n += gauss;
					}
				}
			}
		}

		std::cout << std::endl;
	}

	for(int y=0;y<mat.rows;++y)
		for(int x=0;x<mat.cols;++x)
			for(int a=0;a<3;++a)
				if(norm.ptr<float>(y,x)[a] > 0.0f)
					mat.ptr<float>(y,x)[a] /= norm.ptr<float>(y,x)[a];

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pattern, "pattern", possumwood::opencv::LightfieldPattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_width, "size/width", 300u);
	meta.addAttribute(a_height, "size/height", 300u);
	meta.addAttribute(a_uvScale, "uv_scale", 0.0f);
	meta.addAttribute(a_sigma2, "sigma2", 0.1f);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_pattern, a_out);
	meta.addInfluence(a_width, a_out);
	meta.addInfluence(a_height, a_out);
	meta.addInfluence(a_uvScale, a_out);
	meta.addInfluence(a_sigma2, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/gaussian_refocus", init);

}
