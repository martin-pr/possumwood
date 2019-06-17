#include <boost/filesystem.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "rectangles.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<std::vector<cv::Rect>> a_features;

dependency_graph::State compute(dependency_graph::Values& data) {
	if(!boost::filesystem::exists(data.get(a_filename).filename()))
		throw std::runtime_error("Filename " + data.get(a_filename).filename().string() + " not found");

	cv::CascadeClassifier classifier(data.get(a_filename).filename().string());

	std::vector<cv::Rect> features;
	std::vector<int> numDetections;
	classifier.detectMultiScale(*data.get(a_frame), features, numDetections);

	data.set(a_features, features);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_frame, "frame");
	meta.addAttribute(a_filename, "cascade_filename");
	meta.addAttribute(a_features, "features");

	meta.addInfluence(a_frame, a_features);
	meta.addInfluence(a_filename, a_features);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/clasifier/cascade", init);

}
