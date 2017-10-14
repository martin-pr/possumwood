#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/shader.h"
#include "ui/shader_editor.h"

namespace {

dependency_graph::InAttr<std::string> a_src;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::VertexShader>> a_shader;

class Editor : public possumwood::ShaderEditor {
	public:
		Editor() : ShaderEditor(a_src) {
		}

};

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const std::string& src = data.get(a_src);

	std::unique_ptr<possumwood::VertexShader> shader(new possumwood::VertexShader());

	shader->compile(src);
	result = shader->state();

	if(!shader->state().errored())
		data.set(a_shader, std::shared_ptr<const possumwood::VertexShader>(shader.release()));
	else
		data.set(a_shader, std::shared_ptr<const possumwood::VertexShader>());

	return result;
}

static const std::string defaultSrc =
	"#version 330\n"
	"\n"
	"// input position from the CPU\n"
	"in vec3 position;\n"
	"\n"
	"// near and far per-vertex world positions, useable for raytracing in the fragment shader\n"
	"in vec3 iNearPositionVert;\n"
	"in vec3 iFarPositionVert;\n"
	"out vec3 iNearPosition;\n"
	"out vec3 iFarPosition;\n"
	"\n"
	"void main() {\n"
	"	// do not do any transformation - this should lead to a single quad covering the whole viewport\n"
	"	gl_Position = vec4(position.x, position.y, position.z, 1); \n"
	"	// just pass the near and far positions - they'll get linearly interpolated\n"
	"	iNearPosition = iNearPositionVert;\n"
	"	iFarPosition = iFarPositionVert;\n"
	"}";


void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source", defaultSrc);
	meta.addAttribute(a_shader, "shader");

	meta.addInfluence(a_src, a_shader);

	meta.setCompute(&compute);
	meta.setEditor<Editor>();
}

possumwood::NodeImplementation s_impl("shaders/vertex_shader", init);

}
