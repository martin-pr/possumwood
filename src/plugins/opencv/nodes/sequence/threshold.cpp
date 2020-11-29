#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<float> a_threshold, a_maxVal;
dependency_graph::InAttr<possumwood::Enum> a_type;
dependency_graph::InAttr<bool> a_otsu, a_triangle;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

int modeToEnum(const std::string& mode) {
	if(mode == "THRESH_BINARY")
		return cv::THRESH_BINARY;
	else if(mode == "THRESH_BINARY_INV")
		return cv::THRESH_BINARY_INV;
	else if(mode == "THRESH_TRUNC")
		return cv::THRESH_TRUNC;
	else if(mode == "THRESH_TOZERO")
		return cv::THRESH_TOZERO;
	else if(mode == "THRESH_TOZERO_INV")
		return cv::THRESH_TOZERO_INV;
	else if(mode == "THRESH_MASK")
		return cv::THRESH_MASK;

	throw std::runtime_error("Unknown conversion mode " + mode);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& in = data.get(a_inSequence);
	possumwood::opencv::Sequence out;

	int mode = modeToEnum(data.get(a_type).value());
	if(data.get(a_otsu))
		mode |= cv::THRESH_OTSU;
	if(data.get(a_triangle))
		mode |= cv::THRESH_TRIANGLE;

	tbb::task_group group;

	for(auto it = in.begin(); it != in.end(); ++it) {
		group.run([&data, &mode, &out, it]() {
			cv::Mat m;
			cv::threshold(it->second, m, data.get(a_threshold), data.get(a_maxVal), mode);
			out[it->first] = std::move(m);
		});
	}

	group.wait();

	data.set(a_outSequence, out);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_frame");
	meta.addAttribute(a_threshold, "threshold", 127.0f);
	meta.addAttribute(a_maxVal, "max_val", 255.0f);
	meta.addAttribute(a_type, "type",
	                  possumwood::Enum({"THRESH_BINARY", "THRESH_BINARY_INV", "THRESH_TRUNC", "THRESH_TOZERO",
	                                    "THRESH_TOZERO_INV", "THRESH_MASK"}));
	meta.addAttribute(a_otsu, "use_otsu", false);
	meta.addAttribute(a_triangle, "use_triangle", false);
	meta.addAttribute(a_outSequence, "out_frame");

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_threshold, a_outSequence);
	meta.addInfluence(a_maxVal, a_outSequence);
	meta.addInfluence(a_type, a_outSequence);
	meta.addInfluence(a_otsu, a_outSequence);
	meta.addInfluence(a_triangle, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/threshold", init);

}  // namespace
