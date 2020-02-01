#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <render/datatypes/uniforms.inl>
#include <possumwood_sdk/datatypes/enum.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<std::string> a_name;
dependency_graph::InAttr<possumwood::opencv::Frame> a_value;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<possumwood::Uniforms> a_inUniforms;
dependency_graph::OutAttr<possumwood::Uniforms> a_outUniforms;


dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	possumwood::Uniforms uniforms = data.get(a_inUniforms);

	const possumwood::opencv::Frame& value = data.get(a_value);
	const cv::Mat& mat = *value;

	possumwood::Texture::Interpolation interpolation = possumwood::Texture::kLinear;
	if(data.get(a_mode).value() == "Nearest")
		interpolation = possumwood::Texture::kNearest;

	if(mat.type() == CV_8UC1)
		uniforms.addTexture(
			data.get(a_name),
			mat.data,
			mat.cols,
			mat.rows,
			possumwood::Texture::Format(1, possumwood::Texture::kGray, interpolation) // tightly packed
		);

	else if(mat.type() == CV_8UC3)
		uniforms.addTexture(
			data.get(a_name),
			mat.data,
			mat.cols,
			mat.rows,
			possumwood::Texture::Format(1, possumwood::Texture::kBGR, interpolation) // tightly packed
		);

	else if(mat.type() == CV_32FC3)
		uniforms.addTexture(
			data.get(a_name),
			(float*)mat.data,
			mat.cols,
			mat.rows,
			possumwood::Texture::Format(1, possumwood::Texture::kBGR, interpolation) // tightly packed
		);

	else
		throw std::runtime_error("Unsupported data type " + possumwood::opencv::type2str(mat.type()) + " - render/uniforms/opencv_texture needs extending to support this type!");

	data.set(a_outUniforms, uniforms);

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_name, "name", std::string("image"));
	meta.addAttribute(a_value, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"Linear", "Nearest"}));
	meta.addAttribute(a_inUniforms, "in_uniforms", possumwood::Uniforms(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outUniforms, "out_uniforms", possumwood::Uniforms(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_name, a_outUniforms);
	meta.addInfluence(a_value, a_outUniforms);
	meta.addInfluence(a_mode, a_outUniforms);
	meta.addInfluence(a_inUniforms, a_outUniforms);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("render/uniforms/opencv_texture", init);

}
