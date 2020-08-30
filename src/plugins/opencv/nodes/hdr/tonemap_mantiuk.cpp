#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/photo.hpp>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_gamma, a_saturation, a_scale;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Ptr<cv::TonemapMantiuk> tonemap = cv::createTonemapMantiuk();

	tonemap->setGamma(data.get(a_gamma));
	tonemap->setSaturation(data.get(a_saturation));
	tonemap->setScale(data.get(a_scale));

	cv::Mat mat;
	tonemap->process(*data.get(a_inFrame), mat);

	data.set(a_outFrame, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	cv::Ptr<cv::TonemapMantiuk> dummy = cv::createTonemapMantiuk();

	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_gamma, "gamma", dummy->getGamma());
	meta.addAttribute(a_saturation, "saturation", dummy->getSaturation());
	meta.addAttribute(a_scale, "scale", dummy->getScale());
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_gamma, a_outFrame);
	meta.addInfluence(a_saturation, a_outFrame);
	meta.addInfluence(a_scale, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/hdr/tonemap_mantiuk", init);

}  // namespace