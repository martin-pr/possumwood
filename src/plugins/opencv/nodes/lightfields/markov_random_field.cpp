#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include <lightfields/mrf.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in, a_confidence;
dependency_graph::InAttr<float> a_inputsWeight, a_flatnessWeight, a_smoothnessWeight;
dependency_graph::InAttr<unsigned> a_iterationLimit;
dependency_graph::InAttr<possumwood::Enum> a_method;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

const std::vector<std::pair<std::string, int>>& methods() {
	static std::vector<std::pair<std::string, int>> s_methods;

	if(s_methods.empty()) {
		for(auto& m : lightfields::Neighbours::types())
			s_methods.push_back(std::make_pair("ICM energy optimisation, " + m.first, m.second));

		for(auto& m : lightfields::Neighbours::types())
			s_methods.push_back(std::make_pair("PMF propagation, " + m.first, m.second + 20));

		for(auto& m : lightfields::Neighbours::types())
			s_methods.push_back(std::make_pair("Gaussian PDF propagation, " + m.first, m.second + 40));
	}

	return s_methods;
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_in);
	const cv::Mat& confidence = *data.get(a_confidence);

	if(in.rows != confidence.rows || in.cols != confidence.cols)
		throw std::runtime_error("Input values and confidence needs to have the same size.");

	if(in.type() != CV_8UC1)
		throw std::runtime_error("Input needs to be of type CV_8UC1.");

	if(confidence.type() != CV_32FC1)
		throw std::runtime_error("Confidence needs to be of type CV_32FC1.");

	lightfields::MRF mrf(lightfields::V2i(in.cols, in.rows));

	tbb::parallel_for(0, in.rows, [&](int y) {
		for(int x=0; x<in.cols; ++x) {
			auto& val = mrf[lightfields::V2i(x, y)];
			val.confidence = confidence.at<float>(y, x);
			val.value = in.at<unsigned char>(y, x);
		}
	});

	std::unique_ptr<lightfields::Neighbours> neighbours =
		lightfields::Neighbours::create(lightfields::Neighbours::Type(data.get(a_method).intValue() % 20), lightfields::V2i(in.cols, in.rows));

	cv::Mat result;
	if(data.get(a_method).intValue() < 20)
		result = lightfields::MRF::solveICM(mrf, data.get(a_inputsWeight), data.get(a_flatnessWeight), data.get(a_smoothnessWeight), data.get(a_iterationLimit), *neighbours);
	else if(data.get(a_method).intValue() < 40)
		result = lightfields::MRF::solvePropagation(mrf, data.get(a_inputsWeight), data.get(a_flatnessWeight), data.get(a_smoothnessWeight), data.get(a_iterationLimit), *neighbours);
	else
		result = lightfields::MRF::solvePDF(mrf, data.get(a_inputsWeight), data.get(a_flatnessWeight), data.get(a_smoothnessWeight), data.get(a_iterationLimit), *neighbours);

	data.set(a_out, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_confidence, "confidence", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inputsWeight, "weights/inputs", 1.0f);
	meta.addAttribute(a_flatnessWeight, "weights/flatness", 2.0f);
	meta.addAttribute(a_smoothnessWeight, "weights/smoothness", 2.0f);
	meta.addAttribute(a_iterationLimit, "iterations_limit", 10u);
	meta.addAttribute(a_method, "method", possumwood::Enum(methods().begin(), methods().end()));
	meta.addAttribute(a_out, "out", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_confidence, a_out);
	meta.addInfluence(a_inputsWeight, a_out);
	meta.addInfluence(a_flatnessWeight, a_out);
	meta.addInfluence(a_smoothnessWeight, a_out);
	meta.addInfluence(a_iterationLimit, a_out);
	meta.addInfluence(a_method, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/markov_random_field", init);

}
