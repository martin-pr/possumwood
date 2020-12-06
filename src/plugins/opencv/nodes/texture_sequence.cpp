#include <GL/glew.h>
#include <GL/glu.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <render/datatypes/uniforms.inl>

#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<std::string> a_name;
dependency_graph::InAttr<possumwood::opencv::Sequence> a_value;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<possumwood::Uniforms> a_inUniforms;
dependency_graph::OutAttr<possumwood::Uniforms> a_outUniforms;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	possumwood::Uniforms uniforms = data.get(a_inUniforms);

	possumwood::Texture::Interpolation interpolation = possumwood::Texture::kLinear;
	if(data.get(a_mode).value() == "Nearest")
		interpolation = possumwood::Texture::kNearest;

	const possumwood::opencv::Sequence& value = data.get(a_value);

	if(value.meta().depth == CV_8U) {
		std::vector<const unsigned char*> frames;
		for(auto& f : value)
			frames.push_back(f.second.data);

		if(value.meta().type == CV_8UC1)
			uniforms.addTextureArray(
			    data.get(a_name), frames, value.meta().cols, value.meta().rows,
			    possumwood::Texture::Format(1, possumwood::Texture::kGray, interpolation)  // tightly packed
			);
		else if(value.meta().type == CV_8UC3)
			uniforms.addTextureArray(
			    data.get(a_name), frames, value.meta().cols, value.meta().rows,
			    possumwood::Texture::Format(1, possumwood::Texture::kBGR, interpolation)  // tightly packed
			);
		else {
			std::stringstream ss;
			ss << "Unsupported channel count (" << value.meta().channels
			   << ") - texture sequence only supports 1 or 3 unsigned 8bit int channels.";
			throw std::runtime_error(ss.str());
		}

		const unsigned count = frames.size();
		uniforms.addUniform<unsigned>(
		    data.get(a_name) + "_count", 1, possumwood::Uniforms::kStatic,
		    [count](unsigned* value, std::size_t, const possumwood::ViewportState&) { *value = count; });

		std::vector<Imath::V2i> indices;
		for(auto& f : value)
			indices.push_back(f.first);
		uniforms.addUniform<Imath::V2i>(data.get(a_name) + "_indices", count, possumwood::Uniforms::kStatic,
		                                [indices](Imath::V2i* value, std::size_t, const possumwood::ViewportState&) {
			                                std::size_t ctr = 0;
			                                for(auto& v : indices)
				                                value[ctr++] = v;
		                                });
	}

	else
		throw std::runtime_error("Unsupported data type " + possumwood::opencv::type2str(value.meta().type) +
		                         " - render/uniforms/opencv_texture needs extending to support this type!");

	data.set(a_outUniforms, uniforms);

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_name, "name", std::string("image"));
	meta.addAttribute(a_value, "sequence");
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"Linear", "Nearest"}));
	meta.addAttribute(a_inUniforms, "in_uniforms", possumwood::Uniforms(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outUniforms, "out_uniforms", possumwood::Uniforms(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_name, a_outUniforms);
	meta.addInfluence(a_value, a_outUniforms);
	meta.addInfluence(a_mode, a_outUniforms);
	meta.addInfluence(a_inUniforms, a_outUniforms);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("render/uniforms/opencv_texture_sequence", init);

}  // namespace
