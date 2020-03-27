#include <possumwood_sdk/node_implementation.h>

#include <actions/traits.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "tools.h"
#include "frame.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat m = *data.get(a_in).clone();

	if(m.rows > 0 && m.cols > 0) {
		if(data.get(a_mode).value() == "Min-max") {
			float min = m.at<float>(0,0);
			float max = m.at<float>(0,0);

			for(int y=0;y<m.rows;++y)
				for(int x=0;x<m.cols;++x) {
					const float current = m.at<float>(y, x);
					min = std::min(min, current);
					max = std::max(max, current);
				}

			for(int y=0;y<m.rows;++y)
				for(int x=0;x<m.cols;++x) {
					float& current = m.at<float>(y, x);
					current = (current - min) / (max - min);
				}
		}

		else if(data.get(a_mode).value() == "Max") {
			float max = m.at<float>(0,0);

			for(int y=0;y<m.rows;++y)
				for(int x=0;x<m.cols;++x) {
					const float current = m.at<float>(y, x);
					max = std::max(max, current);
				}

			for(int y=0;y<m.rows;++y)
				for(int x=0;x<m.cols;++x) {
					float& current = m.at<float>(y, x);
					current = current / max;
				}
		}
	}

	data.set(a_out, possumwood::opencv::Frame(m));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"Min-max", "Max"}));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_mode, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/normalize", init);

}
