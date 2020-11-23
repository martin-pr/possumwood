#include <actions/traits.h>
#include <lightfields/samples.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "lightfields.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<lightfields::Samples> a_samples;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Samples samples = data.get(a_samples);

	cv::Mat mat(samples.sensorSize()[0], samples.sensorSize()[1], CV_32FC1);

	tbb::parallel_for(tbb::blocked_range<lightfields::Samples::const_iterator>(samples.begin(), samples.end()),
	                  [&](const tbb::blocked_range<lightfields::Samples::const_iterator>& range) {
		                  for(const lightfields::Samples::Sample& sample : range) {
			                  float* color = mat.ptr<float>(sample.xy.y, sample.xy.x);
			                  float current = sample.uv[0] * sample.uv[0] + sample.uv[1] * sample.uv[1];
			                  if(current <= 1.0f)
				                  color[0] = (1.0f - current);
			                  else
				                  color[0] = 0.0f;
		                  }
	                  });

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}  // namespace

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_samples, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/draw_samples", init);

}  // namespace
