#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>
#include <tbb/parallel_for.h>

#include <actions/traits.h>

#include <lightfields/samples.h>

#include "frame.h"
#include "tools.h"
#include "lightfields.h"

namespace {

dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Samples samples = data.get(a_samples);

	cv::Mat mat(samples.sensorSize()[0], samples.sensorSize()[1], CV_32FC1);

	tbb::parallel_for(0, mat.rows, [&](int y) {
		for(auto it = samples.begin(y); it != samples.end(y); ++it) {
			float* color = mat.ptr<float>(it->source[1], it->source[0]);
			float current = it->uv[0]*it->uv[0] + it->uv[1]*it->uv[1];
			if(current <= 1.0f)
				color[0] = (1.0f-current);
			else
				color[0] = 0.0f;
		}
	});

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_samples, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/draw_samples", init);

}
