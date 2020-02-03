#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>
#include <tbb/parallel_for.h>

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>

#include <maths/io/vec2.h>

#include "frame.h"
#include "lightfield_pattern.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<lightfields::Pattern> a_pattern;
dependency_graph::InAttr<unsigned> a_xRes, a_yRes;
dependency_graph::InAttr<possumwood::Enum> a_xMode, a_yMode;
dependency_graph::InAttr<Imath::V2f> a_xRange, a_yRange, a_uRange, a_vRange;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

int dimension(const std::string& d) {
	if(d == "x")
		return 0;
	else if(d == "y")
		return 1;
	else if(d == "u")
		return 2;
	else if(d == "v")
		return 3;

	throw std::runtime_error("Unknown dimension " + d);
}

bool normalize(float& value, const Imath::V2f& range) {
	value = (value - range[0]) / (range[1] - range[0]);

	return value >= 0.0f && value < 1.0f;
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Pattern pattern = data.get(a_pattern);

	const cv::Mat in = *data.get(a_in);
	if(in.type() != CV_32FC1)
		throw std::runtime_error("Expected a single channel float input image");

	// integrate into an EPI image
	cv::Mat result = cv::Mat::zeros(data.get(a_yRes), data.get(a_xRes), CV_32FC3);
	cv::Mat norm = cv::Mat::zeros(data.get(a_yRes), data.get(a_xRes), CV_16UC3);

	if(Imath::V2i(in.cols, in.rows) != pattern.sensorResolution())
		throw std::runtime_error("Sensor resolution and input image sizes don't match!");

	const int dimx = dimension(data.get(a_xMode).value());
	const int dimy = dimension(data.get(a_yMode).value());

	const Imath::V2f xRange = data.get(a_xRange);
	const Imath::V2f yRange = data.get(a_yRange);
	const Imath::V2f uRange = data.get(a_uRange);
	const Imath::V2f vRange = data.get(a_vRange);

	tbb::parallel_for(0, pattern.sensorResolution().y, [&](int y) {
		for(int x=0; x<pattern.sensorResolution().x; ++x) {
			lightfields::Pattern::Sample sample = pattern.sample(Imath::V2f(x, y));

			// scale x and y to be in range 0..1
			sample.lensCenter[0] /= pattern.sensorResolution().x;
			sample.lensCenter[1] /= pattern.sensorResolution().y;

			// test if the sample is in the expected range

			// handwired lenslet ranges
			bool inRange = ((sample.offset[0]*sample.offset[0] + sample.offset[1]*sample.offset[1]) <= 1.0f);

			// and individual sample ranges
			inRange &= normalize(sample.lensCenter[0], xRange);
			inRange &= normalize(sample.lensCenter[1], yRange);
			inRange &= normalize(sample.offset[0], uRange);
			inRange &= normalize(sample.offset[1], vRange);

			if(inRange) {
				// actual coordinates of the pixel
				int xcoord, ycoord;

				if(dimx <= 1)
					xcoord = std::min((int)floor(sample.lensCenter[dimx] * (float)result.cols), result.cols-1);
				else
					xcoord = std::min((int)floor(sample.offset[dimx-2] * (float)result.cols), result.cols-1);

				if(dimy <= 1)
					ycoord = std::min((int)floor(sample.lensCenter[dimy] * (float)result.rows), result.rows-1);
				else
					ycoord = std::min((int)floor(sample.offset[dimy-2] * (float)result.rows), result.rows-1);

				assert(xcoord >= 0 && xcoord < result.cols);
				assert(ycoord >= 0 && ycoord < result.rows);

				// bayern pattern (hardcoded)
				const int bayern = x%2 + y%2;

				// and the integration
				result.ptr<float>(ycoord, xcoord)[bayern] += in.at<float>(y, x);
				norm.ptr<uint16_t>(ycoord, xcoord)[bayern] += 1;
			}
		}
	});

	tbb::parallel_for(0, result.rows, [&](int y) {
		for(int x=0; x<result.cols; ++x)
			for(int channel=0; channel<3; ++channel)
				if(norm.ptr<uint16_t>(y, x)[channel] > 0)
					result.ptr<float>(y, x)[channel] /= norm.ptr<uint16_t>(y, x)[channel];
	});

	data.set(a_out, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_image", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pattern, "pattern", lightfields::Pattern(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_xRes, "x/resolution", 300u);
	meta.addAttribute(a_xMode, "x/mode", possumwood::Enum({"x", "y", "u", "v"}, 0));
	meta.addAttribute(a_yRes, "y/resolution", 20u);
	meta.addAttribute(a_yMode, "y/mode", possumwood::Enum({"x", "y", "u", "v"}, 3));
	meta.addAttribute(a_xRange, "range/x", Imath::V2f(0, 1));
	meta.addAttribute(a_yRange, "range/y", Imath::V2f(0, 1));
	meta.addAttribute(a_uRange, "range/u", Imath::V2f(-1, 1));
	meta.addAttribute(a_vRange, "range/v", Imath::V2f(-1, 1));
	meta.addAttribute(a_out, "out_image", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_pattern, a_out);
	meta.addInfluence(a_xRes, a_out);
	meta.addInfluence(a_yRes, a_out);
	meta.addInfluence(a_xMode, a_out);
	meta.addInfluence(a_yMode, a_out);
	meta.addInfluence(a_xRange, a_out);
	meta.addInfluence(a_yRange, a_out);
	meta.addInfluence(a_uRange, a_out);
	meta.addInfluence(a_vRange, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/epi", init);

}
