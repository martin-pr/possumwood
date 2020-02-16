#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include <lightfields/samples.h>
#include <lightfields/nearest_integration.h>

#include "maths/io/vec2.h"
#include "frame.h"
#include "tools.h"
#include "lightfields.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out, a_norm, a_correspondence;

dependency_graph::State compute(dependency_graph::Values& data) {
	if((*data.get(a_in)).type() != CV_32FC1 && (*data.get(a_in)).type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	// integration
	auto tmp = lightfields::nearest::integrate(data.get(a_samples), data.get(a_size), *data.get(a_in));

	data.set(a_out, possumwood::opencv::Frame(tmp.average));
	data.set(a_norm, possumwood::opencv::Frame(tmp.samples));

	const cv::Mat& input = *data.get(a_in);
	const lightfields::Samples& samples = data.get(a_samples);

	const unsigned width = data.get(a_size)[0];
	const unsigned height = data.get(a_size)[1];

	// computing correspondence
	cv::Mat corresp = cv::Mat::zeros(height, width, CV_32FC1);

	const float x_scale = (float)width / (float)samples.sensorSize()[0];
	const float y_scale = (float)height / (float)samples.sensorSize()[1];

	tbb::parallel_for(0, input.rows, [&](int y) {
		const auto end = samples.end(y);
		const auto begin = samples.begin(y);
		assert(begin <= end);

		for(auto it = begin; it != end; ++it) {
			const float target_x = it->xy[0] * x_scale;
			const float target_y = it->xy[1] * y_scale;

			if((floor(target_x) >= 0) && (floor(target_y) >= 0) && (floor(target_x) < width) && (floor(target_y) < height)) {
				float* target = corresp.ptr<float>(floor(target_y), floor(target_x));
				const float* value = input.ptr<float>(it->source[1], it->source[0]);
				const float* ave = tmp.average.ptr<float>(target_y, target_x);
				const uint16_t* n = tmp.samples.ptr<uint16_t>(target_y, target_x);

				if(input.channels() == 3)
					for(int c=0; c<3; ++c)
						*target += pow(value[c] - ave[c], 2) / (float)n[c];
				else
					*target += pow(*value - ave[it->color], 2) / (float)n[it->color];
			}
		}
	});

	data.set(a_correspondence, possumwood::opencv::Frame(corresp));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(300u, 300u));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_norm, "sample_count", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_correspondence, "correspondence", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_size, a_out);

	meta.addInfluence(a_in, a_norm);
	meta.addInfluence(a_samples, a_norm);
	meta.addInfluence(a_size, a_norm);

	meta.addInfluence(a_in, a_correspondence);
	meta.addInfluence(a_samples, a_correspondence);
	meta.addInfluence(a_size, a_correspondence);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_nearest", init);

}
