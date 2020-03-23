#include <possumwood_sdk/node_implementation.h>

#include <actions/traits.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "tools.h"
#include "frame.h"

namespace {

static const std::vector<std::pair<std::string, int>> s_modes {{
	std::pair<std::string, int>("Min-max", cv::NORM_MINMAX),
	std::pair<std::string, int>("Infinity norm", cv::NORM_INF),
	std::pair<std::string, int>("L1 norm", cv::NORM_L1),
	std::pair<std::string, int>("L2 norm", cv::NORM_L2),
	std::pair<std::string, int>("L2 square norm", cv::NORM_L2SQR),
	std::pair<std::string, int>("Hamming", cv::NORM_HAMMING),
	std::pair<std::string, int>("Hamming squared", cv::NORM_HAMMING2),
}};

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;
	cv::normalize(*data.get(a_in), result, 1, 0, data.get(a_mode).intValue());

	data.set(a_out, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_modes.begin(), s_modes.end()));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_mode, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/normalize", init);

}
