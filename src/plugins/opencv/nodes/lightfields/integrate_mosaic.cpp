#include <actions/traits.h>
#include <lightfields/samples.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "lightfields.h"
#include "maths/io/vec2.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::InAttr<unsigned> a_elements;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out, a_norm;

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Samples samples = data.get(a_samples);

	const unsigned width = data.get(a_size)[0];
	const unsigned height = data.get(a_size)[1];
	const unsigned elements = data.get(a_elements);

	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	cv::Mat mat = cv::Mat::zeros(height * elements, width * elements, CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(height * elements, width * elements, CV_16UC3);

	tbb::parallel_for(tbb::blocked_range<lightfields::Samples::const_iterator>(samples.begin(), samples.end()),
	                  [&](const tbb::blocked_range<lightfields::Samples::const_iterator> range) {
		                  for(const lightfields::Samples::Sample& sample : range) {
			                  const double uv_magnitude_2 = sample.uv[0] * sample.uv[0] + sample.uv[1] * sample.uv[1];
			                  if(uv_magnitude_2 < 1.0) {
				                  float target_x = sample.xy[0] / (float)samples.sensorSize()[0] * (float)width;
				                  float target_y = sample.xy[1] / (float)samples.sensorSize()[1] * (float)height;

				                  target_x = std::min(target_x, (float)(width - 1));
				                  target_y = std::min(target_y, (float)(height - 1));

				                  target_x = std::max(target_x, 0.0f);
				                  target_y = std::max(target_y, 0.0f);

				                  target_x += floor((sample.uv[0] + 1.0) / 2.0 * (float)elements) * (float)width;
				                  target_y += floor((sample.uv[1] + 1.0) / 2.0 * (float)elements) * (float)height;

				                  float* color = mat.ptr<float>(floor(target_y), floor(target_x));
				                  uint16_t* n = norm.ptr<uint16_t>(floor(target_y), floor(target_x));

				                  if(sample.color == lightfields::Samples::kRGB)
					                  for(int c = 0; c < 3; ++c) {
						                  color[c] += sample.value[c];
						                  ++n[c];
					                  }
				                  else {
					                  color[sample.color] += sample.value[sample.color];
					                  ++n[sample.color];
				                  }
			                  }
		                  }
	                  });

	tbb::parallel_for(0, mat.rows, [&](int y) {
		for(int x = 0; x < mat.cols; ++x)
			for(int a = 0; a < 3; ++a)
				if(norm.ptr<uint16_t>(y, x)[a] > 0.0f)
					mat.ptr<float>(y, x)[a] /= (float)norm.ptr<uint16_t>(y, x)[a];
	});

	data.set(a_out, possumwood::opencv::Frame(mat));
	data.set(a_norm, possumwood::opencv::Frame(norm));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(300u, 300u));
	meta.addAttribute(a_elements, "elements", 5u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_norm, "sample_count", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_size, a_out);
	meta.addInfluence(a_elements, a_out);

	meta.addInfluence(a_samples, a_norm);
	meta.addInfluence(a_size, a_out);
	meta.addInfluence(a_elements, a_norm);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_mosaic", init);

}  // namespace
