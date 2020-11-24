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
dependency_graph::InAttr<float> a_threshold;
dependency_graph::InAttr<unsigned> a_maxIter;
dependency_graph::OutAttr<possumwood::opencv::CameraResponse> a_response;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Ptr<cv::CalibrateRobertson> calibrate = cv::createCalibrateRobertson();

	calibrate->setThreshold(data.get(a_threshold));
	calibrate->setMaxIter(data.get(a_maxIter));

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

	if(inputs.size() != exposures.size())
		throw std::runtime_error("Inputs and exif data counts need to match!");

	cv::Mat mat;
	if(!inputs.empty() && !exposures.empty())
		calibrate->process(inputs, mat, exposures);

	data.set(a_response, possumwood::opencv::CameraResponse(mat, exposures));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	cv::Ptr<cv::CalibrateRobertson> dummy = cv::createCalibrateRobertson();

	meta.addAttribute(a_sequence, "image_sequence", possumwood::opencv::Sequence());
	meta.addAttribute(a_exifSequence, "exif_sequence", possumwood::opencv::ExifSequence());
	meta.addAttribute(a_threshold, "threshold", dummy->getThreshold());
	meta.addAttribute(a_maxIter, "max_iter", (unsigned)dummy->getMaxIter());
	meta.addAttribute(a_response, "camera_response", possumwood::opencv::CameraResponse());

	meta.addInfluence(a_sequence, a_response);
	meta.addInfluence(a_exifSequence, a_response);
	meta.addInfluence(a_threshold, a_response);
	meta.addInfluence(a_maxIter, a_response);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/hdr/calibrate_robertson", init);

}  // namespace
