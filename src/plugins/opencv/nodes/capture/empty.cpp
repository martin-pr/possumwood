#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "maths/io/vec2.h"
#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::InAttr<float> a_color;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

int modeToEnum(const std::string& mode) {
	if(mode == "CV_8UC1")
		return CV_8UC1;
	else if(mode == "CV_8UC2")
		return CV_8UC2;
	else if(mode == "CV_8UC3")
		return CV_8UC3;
	else if(mode == "CV_8UC4")
		return CV_8UC4;
	else if(mode == "CV_8SC1")
		return CV_8SC1;
	else if(mode == "CV_8SC2")
		return CV_8SC2;
	else if(mode == "CV_8SC3")
		return CV_8SC3;
	else if(mode == "CV_8SC4")
		return CV_8SC4;
	else if(mode == "CV_16UC1")
		return CV_16UC1;
	else if(mode == "CV_16UC2")
		return CV_16UC2;
	else if(mode == "CV_16UC3")
		return CV_16UC3;
	else if(mode == "CV_16UC4")
		return CV_16UC4;
	else if(mode == "CV_16SC1")
		return CV_16SC1;
	else if(mode == "CV_16SC2")
		return CV_16SC2;
	else if(mode == "CV_16SC3")
		return CV_16SC3;
	else if(mode == "CV_16SC4")
		return CV_16SC4;
	else if(mode == "CV_32SC1")
		return CV_32SC1;
	else if(mode == "CV_32SC2")
		return CV_32SC2;
	else if(mode == "CV_32SC3")
		return CV_32SC3;
	else if(mode == "CV_32SC4")
		return CV_32SC4;
	else if(mode == "CV_32FC1")
		return CV_32FC1;
	else if(mode == "CV_32FC2")
		return CV_32FC2;
	else if(mode == "CV_32FC3")
		return CV_32FC3;
	else if(mode == "CV_32FC4")
		return CV_32FC4;
	else if(mode == "CV_64FC1")
		return CV_64FC1;
	else if(mode == "CV_64FC2")
		return CV_64FC2;
	else if(mode == "CV_64FC3")
		return CV_64FC3;
	else if(mode == "CV_64FC4")
		return CV_64FC4;

	throw std::runtime_error("Enum conversion error - unknown mode " + mode);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result = cv::Mat(data.get(a_size)[1], data.get(a_size)[0], modeToEnum(data.get(a_mode).value()));
	result = data.get(a_color);

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_mode, "mode",
		possumwood::Enum({
			"CV_8UC1", "CV_8UC2", "CV_8UC3", "CV_8UC4", "CV_8SC1", "CV_8SC2", "CV_8SC3", "CV_8SC4", "CV_16UC1", "CV_16UC2",
			"CV_16UC3", "CV_16UC4", "CV_16SC1", "CV_16SC2", "CV_16SC3", "CV_16SC4", "CV_32SC1", "CV_32SC2", "CV_32SC3",
			"CV_32SC4", "CV_32FC1", "CV_32FC2", "CV_32FC3", "CV_32FC4", "CV_64FC1", "CV_64FC2", "CV_64FC3", "CV_64FC4"
		}));
	meta.addAttribute(a_size, "size");
	meta.addAttribute(a_color, "color");
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_mode, a_outFrame);
	meta.addInfluence(a_size, a_outFrame);
	meta.addInfluence(a_color, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/empty", init);

}
