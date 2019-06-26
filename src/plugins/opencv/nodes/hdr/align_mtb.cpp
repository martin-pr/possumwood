#include <possumwood_sdk/node_implementation.h>

#include <opencv2/photo.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_original, a_toAlign;
dependency_graph::InAttr<bool> a_cut;
dependency_graph::InAttr<unsigned> a_excludeRange, a_maxBits;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat mat;

	if(!data.get(a_original).empty() && !data.get(a_toAlign).empty()) {
		cv::Ptr<cv::AlignMTB> align = cv::createAlignMTB();

		align->setCut(data.get(a_cut));
		align->setExcludeRange(data.get(a_excludeRange));
		align->setMaxBits(data.get(a_maxBits));

		const cv::Mat& original = *data.get(a_original);
		const cv::Mat& toAlign = *data.get(a_toAlign);

		cv::Mat origGray;
		cvtColor(original, origGray, cv::COLOR_BGR2GRAY);

		cv::Mat alignGray;
		cvtColor(toAlign, alignGray, cv::COLOR_BGR2GRAY);

		const cv::Point shift = align->calculateShift(origGray, alignGray);

		align->shiftMat(toAlign, mat, shift);
	}

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	cv::Ptr<cv::AlignMTB> dummy = cv::createAlignMTB();

	meta.addAttribute(a_original, "original", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_toAlign, "to_align", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_cut, "cut", dummy->getCut());
	meta.addAttribute(a_excludeRange, "exclude_range", (unsigned)dummy->getExcludeRange());
	meta.addAttribute(a_maxBits, "max_bits", (unsigned)dummy->getMaxBits());
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_original, a_out);
	meta.addInfluence(a_toAlign, a_out);
	meta.addInfluence(a_cut, a_out);
	meta.addInfluence(a_excludeRange, a_out);
	meta.addInfluence(a_maxBits, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/hdr/align_mtb", init);

}
