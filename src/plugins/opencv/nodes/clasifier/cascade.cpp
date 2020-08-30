#include <actions/traits.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>

#include "frame.h"
#include "rectangles.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::InAttr<float> a_scaleFactor;
dependency_graph::InAttr<unsigned> a_minNeighbors;
dependency_graph::OutAttr<std::vector<cv::Rect>> a_features;

dependency_graph::State compute(dependency_graph::Values& data) {
	if(!boost::filesystem::exists(data.get(a_filename).filename()))
		throw std::runtime_error("Filename " + data.get(a_filename).filename().string() + " not found");

	cv::CascadeClassifier classifier(data.get(a_filename).filename().string());

	std::vector<cv::Rect> features;
	classifier.detectMultiScale(*data.get(a_frame), features, data.get(a_scaleFactor), data.get(a_minNeighbors));

	data.set(a_features, features);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_filename, "cascade_filename");
	meta.addAttribute(a_scaleFactor, "scale_factor", 1.1f);
	meta.addAttribute(a_minNeighbors, "min_neighbors", 3u);
	meta.addAttribute(a_features, "features", std::vector<cv::Rect>(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_frame, a_features);
	meta.addInfluence(a_filename, a_features);
	meta.addInfluence(a_scaleFactor, a_features);
	meta.addInfluence(a_minNeighbors, a_features);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/clasifier/cascade", init);

}  // namespace
