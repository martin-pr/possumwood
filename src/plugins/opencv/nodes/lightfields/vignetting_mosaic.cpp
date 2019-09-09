#include <possumwood_sdk/node_implementation.h>

#include <thread>
#include <mutex>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>

#include "frame.h"
#include "lightfield_vignetting.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::LightfieldVignetting> a_vignetting;
dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
dependency_graph::InAttr<unsigned> a_elements;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::LightfieldVignetting& vignetting = data.get(a_vignetting);

	const unsigned width = data.get(a_size)[0];
	const unsigned height = data.get(a_size)[1];
	const unsigned elements = data.get(a_elements);

	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC1);

	if(data.get(a_mode).value() == "XY-UV") {
		tbb::parallel_for(0u, height, [&](unsigned y) {
			for(unsigned x = 0; x < width; ++x) {
				float xf = (float)x / (float)(width) * (float)elements;
				float yf = (float)y / (float)(height) * (float)elements;

				float uf = ((floor(xf) + 0.5) / (float)(elements) - 0.5) * 2.0;
				float vf = ((floor(yf) + 0.5) / (float)(elements) - 0.5) * 2.0;

				xf = std::fmod(xf, 1.0f);
				yf = std::fmod(yf, 1.0f);

				const double sample = vignetting.sample(cv::Vec4f(xf, yf, uf, vf));

				mat.at<float>(y, x) = sample;
			}
		});
	}
	else {
		tbb::parallel_for(0u, height, [&](unsigned y) {
			for(unsigned x = 0; x < width; ++x) {
				float uf = (float)x / (float)(width) * (float)elements;
				float vf = (float)y / (float)(height) * (float)elements;

				float xf = ((floor(uf) + 0.5) / (float)(elements + 1));
				float yf = ((floor(vf) + 0.5) / (float)(elements + 1));

				uf = (std::fmod(uf, 1.0f) - 0.5) * 2.0;
				vf = (std::fmod(vf, 1.0f) - 0.5) * 2.0;

				const double sample = vignetting.sample(cv::Vec4f(xf, yf, uf, vf));

				mat.at<float>(y, x) = sample;
			}
		});
	}

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_vignetting, "vignetting", possumwood::opencv::LightfieldVignetting(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_size, "size", Imath::Vec2<unsigned>(300u, 300u));
	meta.addAttribute(a_elements, "elements", 5u);
	meta.addAttribute(a_mode, "mode",
		possumwood::Enum({"XY-UV", "UV-XY"}));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_vignetting, a_out);
	meta.addInfluence(a_size, a_out);
	meta.addInfluence(a_elements, a_out);
	meta.addInfluence(a_mode, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/vignetting_mosaic", init);

}


