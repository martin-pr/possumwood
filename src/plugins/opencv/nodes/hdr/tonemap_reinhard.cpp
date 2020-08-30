#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/photo.hpp>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<float> a_gamma, a_intensity, a_lightAdaptation, a_colorAdaptation;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Ptr<cv::TonemapReinhard> tonemap = cv::createTonemapReinhard();

	tonemap->setGamma(data.get(a_gamma));
	tonemap->setIntensity(data.get(a_intensity));
	tonemap->setLightAdaptation(data.get(a_lightAdaptation));
	tonemap->setColorAdaptation(data.get(a_colorAdaptation));

	cv::Mat mat;
	tonemap->process(*data.get(a_inFrame), mat);

	data.set(a_outFrame, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	cv::Ptr<cv::TonemapReinhard> dummy = cv::createTonemapReinhard();

	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_gamma, "gamma", dummy->getGamma());
	meta.addAttribute(a_intensity, "intensity", dummy->getIntensity());
	meta.addAttribute(a_lightAdaptation, "light_adaptation", dummy->getLightAdaptation());
	meta.addAttribute(a_colorAdaptation, "color_adaptation", dummy->getColorAdaptation());
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_gamma, a_outFrame);
	meta.addInfluence(a_intensity, a_outFrame);
	meta.addInfluence(a_lightAdaptation, a_outFrame);
	meta.addInfluence(a_colorAdaptation, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/hdr/tonemap_reinhard", init);

}  // namespace
