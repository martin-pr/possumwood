#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

#include "laplacian.h"
#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<possumwood::Enum> a_kernel;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& inSeq = data.get(a_in);

	if((inSeq.meta().type != CV_32FC1) && (inSeq.meta().type != CV_32FC3))
		throw std::runtime_error("Only works with CV_32FC1 and CV_32FC3");

	possumwood::opencv::Sequence outSeq;

	tbb::task_group group;

	std::size_t i = 0;
	for(auto it = inSeq.begin(); it != inSeq.end(); ++it, ++i) {
		group.run([&data, it, &outSeq]() {
			const cv::Mat out = possumwood::opencv::laplacian(
			    it->second, possumwood::opencv::LaplacianKernel(data.get(a_kernel).intValue()));

			std::vector<cv::Mat> tmp(out.channels());
			cv::split(out, tmp);

			for(auto& m : tmp)
				m = cv::abs(m);

			for(std::size_t a = 1; a < tmp.size(); ++a)
				cv::add(tmp[0], tmp[a], tmp[0]);

			outSeq[it->first] = std::move(tmp[0]);
		});
	}

	group.wait();

	data.set(a_out, outSeq);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_seq", possumwood::opencv::Sequence());
	meta.addAttribute(
	    a_kernel, "kernel",
	    possumwood::Enum(possumwood::opencv::laplacianKernels().begin(), possumwood::opencv::laplacianKernels().end()));
	meta.addAttribute(a_out, "out_seq", possumwood::opencv::Sequence());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_kernel, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/defocus_metric", init);

}  // namespace
