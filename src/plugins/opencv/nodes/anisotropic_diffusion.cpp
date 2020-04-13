#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include <lightfields/neighbours.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<float> a_coefficient, a_step;
dependency_graph::InAttr<unsigned> a_iterationLimit;
dependency_graph::InAttr<possumwood::Enum> a_neighbourhood;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

float c(float val, float k) {
	return 1.0f / (1.0f + (val*val / k*k));
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_in);
	const float k = data.get(a_coefficient);

	if(in.type() != CV_32FC1)
		throw std::runtime_error("Input needs to be of type CV_32FC1.");

	std::unique_ptr<lightfields::Neighbours> neighbours = lightfields::Neighbours::create(
		lightfields::Neighbours::Type(data.get(a_neighbourhood).intValue()), lightfields::V2i(in.cols, in.rows));

	cv::Mat mat = in.clone();
	cv::Mat source = mat.clone();

	for(unsigned it=0; it<data.get(a_iterationLimit); ++it) {
		cv::swap(source, mat);

		tbb::parallel_for(tbb::blocked_range2d<int>(0, mat.rows, 0, mat.cols), [&](const tbb::blocked_range2d<int>& range) {
			for(int y=range.rows().begin(); y != range.rows().end(); ++y)
				for(int x=range.cols().begin(); x != range.cols().end(); ++x) {
					float laplacian = 0.0f;
					float norm = 0.0f;

					neighbours->eval(lightfields::V2i(x, y), [&](const lightfields::V2i& pos, float weight) {
						norm += weight;

						const float diff = source.at<float>(pos.y, pos.x) - source.at<float>(y, x);

						laplacian += diff * c(diff, k) * weight;
					});

					laplacian = laplacian / norm;

					mat.at<float>(y, x) = source.at<float>(y, x) + laplacian * data.get(a_step);
				}
		});
	}

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_coefficient, "coefficient", 0.1f);
	meta.addAttribute(a_step, "step", 0.01f);
	meta.addAttribute(a_iterationLimit, "iterations_limit", 10u);
	meta.addAttribute(a_neighbourhood, "neighbourhood", possumwood::Enum(lightfields::Neighbours::types().begin(), lightfields::Neighbours::types().end()));
	meta.addAttribute(a_out, "out", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_coefficient, a_out);
	meta.addInfluence(a_step, a_out);
	meta.addInfluence(a_iterationLimit, a_out);
	meta.addInfluence(a_neighbourhood, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/anisotropic_diffusion", init);

}
