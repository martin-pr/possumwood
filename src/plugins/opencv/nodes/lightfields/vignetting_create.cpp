#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "lightfield_pattern.h"
#include "lightfield_vignetting.h"
#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const lightfields::Pattern>> a_pattern;
dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<unsigned> a_subdivLevel;
dependency_graph::OutAttr<possumwood::opencv::LightfieldVignetting> a_vignetting;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::shared_ptr<const lightfields::Pattern> pattern = data.get(a_pattern);
	if(pattern == nullptr)
		throw std::runtime_error("Non-empty pattern expected");

	const cv::Mat& in = *data.get(a_in);

	if(in.cols != pattern->sensorResolution()[1] || in.rows != pattern->sensorResolution()[0])
		throw std::runtime_error("Frame and pattern resolution don't match!");

	possumwood::opencv::LightfieldVignetting vignetting(data.get(a_subdivLevel), *pattern, in);

	data.set(a_vignetting, vignetting);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pattern, "pattern", std::shared_ptr<const lightfields::Pattern>(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_subdivLevel, "subdiv_level", 32u);
	meta.addAttribute(a_vignetting, "vignetting", possumwood::opencv::LightfieldVignetting(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_pattern, a_vignetting);
	meta.addInfluence(a_in, a_vignetting);
	meta.addInfluence(a_subdivLevel, a_vignetting);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/vignetting_create", init);

}
