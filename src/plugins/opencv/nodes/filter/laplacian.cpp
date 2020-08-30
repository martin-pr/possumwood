#include "laplacian.h"

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::Enum> a_kernel;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_in);

	if((in.type() != CV_32FC1) && (in.type() != CV_32FC3))
		throw std::runtime_error("Only works with CV_32FC1 and CV_32FC3");

	const cv::Mat out =
	    possumwood::opencv::laplacian(in, possumwood::opencv::LaplacianKernel(data.get(a_kernel).intValue()));

	data.set(a_out, possumwood::opencv::Frame(out));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(
	    a_kernel, "kernel",
	    possumwood::Enum(possumwood::opencv::laplacianKernels().begin(), possumwood::opencv::laplacianKernels().end()));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_kernel, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/filter/laplacian", init);

}  // namespace
