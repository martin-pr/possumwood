#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>

#include <chrono>
#include <thread>

#include <tbb/parallel_for.h>
#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

#include "maths/io/vec2.h"
#include "sequence.h"
#include "tools.h"

namespace {

using namespace std::chrono_literals;

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::InAttr<float> a_offset;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out, a_mask;

void putPixel(std::vector<std::atomic<float>>& colors,
              std::vector<std::atomic<float>>& norms,
              const Imath::V2i& pos,
              int width,
              int height,
              const float* data,
              float weight) {
	if(pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height) {
		std::atomic<float>* color = colors.data() + (pos.y * width + pos.x) * 3;
		std::atomic<float>* n = norms.data() + (pos.y * width + pos.x) * 3;

		for(int a = 0; a < 3; ++a) {
			float exp_color = color[a];
			while(!color[a].compare_exchange_weak(exp_color, exp_color + data[a] * weight))
				std::this_thread::sleep_for(100us);

			float exp_norm = n[a];
			while(!n[a].compare_exchange_weak(exp_norm, exp_norm + weight))
				std::this_thread::sleep_for(100us);
		}
	}
}

void putPixel(std::vector<std::atomic<float>>& colors,
              std::vector<std::atomic<float>>& norms,
              const Imath::V2f& posf,
              int width,
              int height,
              const float* data) {
	// putPixel(colors, norms, Imath::V2i(round(posf.x), round(posf.y)), width, height, data, 1.0f);

	const Imath::V2i pos(floor(posf.x), floor(posf.y));

	const float wx = posf.x - floor(posf.x);
	const float wy = posf.y - floor(posf.y);

	putPixel(colors, norms, Imath::V2i(pos.x, pos.y), width, height, data, (1.0f - wx) * (1.0f - wy));
	putPixel(colors, norms, Imath::V2i(pos.x + 1, pos.y), width, height, data, (wx) * (1.0f - wy));
	putPixel(colors, norms, Imath::V2i(pos.x, pos.y + 1), width, height, data, (1.0f - wx) * wy);
	putPixel(colors, norms, Imath::V2i(pos.x + 1, pos.y + 1), width, height, data, wx * wy);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& input = data.get(a_in);
	if(input.meta().type != CV_32FC3)
		throw std::runtime_error("Only 32-bit 3 channel float format supported on input, " +
		                         possumwood::opencv::type2str(input.meta().type) + " found instead!");

	const int width = data.get(a_size)[0];
	const int height = data.get(a_size)[1];

	const float offset = data.get(a_offset);

	std::vector<std::atomic<float>> mat(height * width * 3);
	std::vector<std::atomic<float>> norm(height * width * 3);

	tbb::task_group group;

	for(auto it = input.begin(); it != input.end(); ++it) {
		group.run([it, &mat, &norm, &offset, &width, &height]() {
			const Imath::V2f offs = Imath::V2f(it->first.x, it->first.y) * (float)offset;
			for(int y = 0; y < it->second.rows; ++y)
				for(int x = 0; x < it->second.cols; ++x) {
					const Imath::V2f posf((float)x / (float)it->second.cols * (float)width + offs.x,
					                      (float)y / (float)it->second.rows * (float)height + offs.y);

					putPixel(mat, norm, posf, width, height, it->second.ptr<float>(y, x));
				}
		});
	}

	group.wait();

	cv::Mat result = cv::Mat::zeros(height, width, CV_32FC3);
	cv::Mat resultNorm = cv::Mat::zeros(height, width, CV_32FC3);

	tbb::parallel_for(0, height, [&](int y) {
		for(int x = 0; x < width; ++x)
			for(int a = 0; a < 3; ++a) {
				const float n = norm[(y * width + x) * 3 + a];
				if(n > 0.0f)
					result.ptr<float>(y, x)[a] = mat[(y * width + x) * 3 + a] / n;
				resultNorm.ptr<float>(y, x)[a] = n;
			}
	});

	data.set(a_out, possumwood::opencv::Frame(result));
	data.set(a_mask, possumwood::opencv::Frame(resultNorm));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "sequence");
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(1000, 1000));
	meta.addAttribute(a_offset, "offset", 3.0f);
	meta.addAttribute(a_out, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mask, "mask", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_size, a_out);
	meta.addInfluence(a_offset, a_out);

	meta.addInfluence(a_in, a_mask);
	meta.addInfluence(a_size, a_mask);
	meta.addInfluence(a_offset, a_mask);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/mosaic_superres", init);

}  // namespace
