#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/photo.hpp>

#include "camera_response.h"
#include "exif_sequence.h"
#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_sequence;
dependency_graph::InAttr<possumwood::opencv::ExifSequence> a_exifSequence;
dependency_graph::InAttr<float> a_lambda;
dependency_graph::InAttr<bool> a_random;
dependency_graph::InAttr<unsigned> a_samples;
dependency_graph::OutAttr<possumwood::opencv::CameraResponse> a_response;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Ptr<cv::CalibrateDebevec> calibrate = cv::createCalibrateDebevec();

	calibrate->setLambda(data.get(a_lambda));
	calibrate->setRandom(data.get(a_random));
	calibrate->setSamples(data.get(a_samples));

	// collect input frames - doesn't copy, just uses shared references
	std::vector<cv::Mat> inputs;
	for(auto& in : data.get(a_sequence))
		inputs.push_back(in);

	// build an array of exposures from exif
	std::vector<float> exposures;
	for(auto& e : data.get(a_exifSequence))
		if(e.valid())
			exposures.push_back(e.exposure() * e.iso() / 100.0f);

	// std::cout << "Exposures:";
	// for(auto& e : data.get(a_exifSequence))
	// 	std::cout << "  " << e.exposure();
	// std::cout << std::endl;

	// std::cout << "F-numbers:";
	// for(auto& e : data.get(a_exifSequence))
	// 	std::cout << "  " << e.f();
	// std::cout << std::endl;

	// std::cout << "ISOs:";
	// for(auto& e : data.get(a_exifSequence))
	// 	std::cout << "  " << e.iso();
	// std::cout << std::endl;

	// std::cout << "Normalized exposures:";
	// for(auto& e : exposures)
	// 	std::cout << "  " << e;
	// std::cout << std::endl;

	cv::Mat mat;

	if(inputs.size() != exposures.size())
		throw std::runtime_error("Inputs and exif data counts need to match!");

	if(!inputs.empty() && !exposures.empty())
		calibrate->process(inputs, mat, exposures);

	data.set(a_response, possumwood::opencv::CameraResponse(mat, exposures));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	cv::Ptr<cv::CalibrateDebevec> dummy = cv::createCalibrateDebevec();

	meta.addAttribute(a_sequence, "image_sequence", possumwood::opencv::Sequence());
	meta.addAttribute(a_exifSequence, "exif_sequence", possumwood::opencv::ExifSequence());
	meta.addAttribute(a_lambda, "lambda", dummy->getLambda());
	meta.addAttribute(a_random, "random", dummy->getRandom());
	meta.addAttribute(a_samples, "samples", (unsigned)dummy->getSamples());
	meta.addAttribute(a_response, "camera_response", possumwood::opencv::CameraResponse());

	meta.addInfluence(a_sequence, a_response);
	meta.addInfluence(a_exifSequence, a_response);
	meta.addInfluence(a_lambda, a_response);
	meta.addInfluence(a_random, a_response);
	meta.addInfluence(a_samples, a_response);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/hdr/calibrate_debevec", init);

}  // namespace
