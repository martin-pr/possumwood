#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "scoped_error_redirect.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<float> a_a, a_b;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int modeToEnum(const std::string& mode) {
	if(mode == "CV_8U")
		return CV_8U;
	else if(mode == "CV_16U")
		return CV_16U;
	else if(mode == "CV_32F")
		return CV_32F;

	throw std::runtime_error("Unknown conversion mode " + mode);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State state;

	cv::Mat result;

	{
		possumwood::opencv::ScopedErrorRedirect errors;
		(*data.get(a_inFrame))
		    .clone()
		    .convertTo(result, modeToEnum(data.get(a_mode).value()), data.get(a_a), data.get(a_b));
		state.append(errors.state());
	}

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return state;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"CV_8U", "CV_16U", "CV_32F"}));
	meta.addAttribute(a_a, "a", 1.0f);
	meta.addAttribute(a_b, "b", 0.0f);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_mode, a_outFrame);
	meta.addInfluence(a_a, a_outFrame);
	meta.addInfluence(a_b, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/convert", init);

}  // namespace
