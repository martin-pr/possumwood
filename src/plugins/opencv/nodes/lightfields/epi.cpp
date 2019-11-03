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

dependency_graph::State compute(dependency_graph::Values& data) {
	const lightfields::Pattern& pattern = data.get(a_pattern);

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

	std::vector<Imath::V2f> ranges;
	ranges.push_back(data.get(a_xRange));
	ranges.push_back(data.get(a_yRange));
	ranges.push_back(data.get(a_uRange));
	ranges.push_back(data.get(a_vRange));

	tbb::parallel_for(0, pattern.sensorResolution().y, [&](int y) {
		for(int x=0; x<pattern.sensorResolution().x; ++x) {
			Imath::V4f sample = pattern.sample(Imath::V2f(x, y));

			// scale x and y to be in range 0..1
			sample[0] /= pattern.sensorResolution().x;
			sample[1] /= pattern.sensorResolution().y;

			// test if the sample is in the expected range
			bool inRange = true;

			// handwired lenslet ranges
			inRange = inRange && ((sample[2]*sample[2] + sample[3]*sample[3]) <= 1.0f);

			for(int d=0;d<4;++d) {
				sample[d] = (sample[d] - ranges[d][0]) / (ranges[d][1] - ranges[d][0]);
				// std::cout <<d << "  " << sample[d] << std::endl;
				if(sample[d] < 0.0f || sample[d] > 1.0f)
					inRange = false;
			}

			if(inRange) {
				// bayern pattern (hardcoded)
				const int bayern = x%2 + y%2;

				// actual coordinates of the pixel
				const int xcoord = std::min((int)floor(sample[dimx] * (float)result.cols), result.cols-1);
				const int ycoord = std::min((int)floor(sample[dimy] * (float)result.rows), result.rows-1);

				assert(xcoord >= 0 && xcoord < result.cols);
				assert(ycoord >= 0 && ycoord < result.rows);

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
