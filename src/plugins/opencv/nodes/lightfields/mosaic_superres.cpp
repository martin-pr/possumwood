#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "maths/io/vec2.h"
#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::InAttr<unsigned> a_mosaic;
dependency_graph::InAttr<float> a_offset;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out, a_mask;

dependency_graph::State compute(dependency_graph::Values& data) {
	const int mosaic = data.get(a_mosaic);

	const cv::Mat input = *data.get(a_in);
	if(input.type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit 3 channel float format supported on input, " + possumwood::opencv::type2str(input.type()) + " found instead!");

	const int width = data.get(a_size)[0];
	const int height = data.get(a_size)[1];

	if(input.rows % mosaic != 0 || input.cols % mosaic != 0)
		throw std::runtime_error("Width or height is not divisible by mosaic count - not a mosaic?");

	const float offset = data.get(a_offset);

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height, width, CV_16UC3);

	const int tile_width = input.cols / mosaic;
	const int tile_height = input.rows / mosaic;

	tbb::parallel_for(0, input.rows, [&](int y) {
		for(int x=0;x<input.cols;++x) {
			float xi = x % tile_width;
			float yi = y % tile_height;

			xi += (float)(x/tile_width - mosaic/2) * offset;
			yi += (float)(y/tile_height - mosaic/2) * offset;

			xi = xi / (float)(tile_width) * (float)width;
			yi = yi / (float)(tile_height) * (float)height;

			const int target_x = (int)round(xi);
			const int target_y = (int)round(yi);

			if(target_x >= 0 && target_x < width && target_y >= 0 && target_y < height) {
				float* color = mat.ptr<float>(target_y, target_x);
				int16_t* n = norm.ptr<int16_t>(target_y, target_x);

				const float* value = input.ptr<float>(y, x);

				for(int a=0;a<3;++a) {
					color[a] += value[a];
					n[a] += 1;
				}
			}
		}
	});

	tbb::parallel_for(0, mat.rows, [&](int y) {
		for(int x=0;x<mat.cols;++x)
			for(int a=0;a<3;++a)
				if(norm.ptr<int16_t>(y,x)[a] > 0)
					mat.ptr<float>(y,x)[a] /= (float)norm.ptr<int16_t>(y,x)[a];
	});

	data.set(a_out, possumwood::opencv::Frame(mat));
	data.set(a_mask, possumwood::opencv::Frame(norm));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(1000,1000));
	meta.addAttribute(a_mosaic, "mosaic", 9u);
	meta.addAttribute(a_offset, "offset", 3.0f);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mask, "mask", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_size, a_out);
	meta.addInfluence(a_mosaic, a_out);
	meta.addInfluence(a_offset, a_out);

	meta.addInfluence(a_in, a_mask);
	meta.addInfluence(a_size, a_mask);
	meta.addInfluence(a_mosaic, a_mask);
	meta.addInfluence(a_offset, a_mask);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/mosaic_superres", init);

}
