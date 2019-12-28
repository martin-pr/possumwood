#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "lightfield_pattern.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<lightfields::Pattern> a_pattern;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Pattern& pattern = data.get(a_pattern);

	cv::Mat mat(pattern.sensorResolution()[0], pattern.sensorResolution()[1], CV_32FC1);

	for(int y=0;y<mat.rows;++y)
		for(int x=0;x<mat.cols;++x) {
			const Imath::V4f value = pattern.sample(Imath::V2i(x, y));

			float* color = mat.ptr<float>(y, x);
			float current = value[2]*value[2] + value[3]*value[3];
			if(current <= 1.0f)
				color[0] += (1.0f-current);
			else
				color[0] = 0.0f;
		}

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_pattern, "pattern", lightfields::Pattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_pattern, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/draw_pattern", init);

}
