#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <tbb/parallel_for.h>

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>

#include "sequence.h"
#include "tools.h"
#include "laplacian.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<possumwood::Enum> a_kernel;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& inSeq = data.get(a_in);

	possumwood::opencv::Sequence outSeq(inSeq.size());

	tbb::parallel_for(std::size_t(0), inSeq.size(), [&](std::size_t i) {
		const cv::Mat& in = *inSeq[i];

		if((in.type() != CV_32FC1) && (in.type() != CV_32FC3))
			throw std::runtime_error("Only works with CV_32FC1 and CV_32FC3");

		const cv::Mat out = possumwood::opencv::laplacian(in, possumwood::opencv::LaplacianKernel(data.get(a_kernel).intValue()));

		std::vector<cv::Mat> tmp(out.channels());
		cv::split(out, tmp);

		for(auto& m : tmp)
			m = cv::abs(m);

		for(std::size_t a=1; a<tmp.size(); ++a)
			cv::add(tmp[0], tmp[a], tmp[0]);

		outSeq[i].mat = tmp[0];
	});

	data.set(a_out, outSeq);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_seq", possumwood::opencv::Sequence());
	meta.addAttribute(a_kernel, "kernel", possumwood::Enum(possumwood::opencv::laplacianKernels().begin(), possumwood::opencv::laplacianKernels().end()));
	meta.addAttribute(a_out, "out_seq", possumwood::opencv::Sequence());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_kernel, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/defocus_metric", init);

}
