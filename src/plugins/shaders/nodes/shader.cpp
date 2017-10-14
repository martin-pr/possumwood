#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/shader.h"
#include "ui/shader_editor.h"

namespace {

dependency_graph::InAttr<std::string> a_src;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::Shader>> a_shader;

class Editor : public possumwood::ShaderEditor {
	public:
		Editor() : ShaderEditor(a_src) {
		}

};

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const std::string& src = data.get(a_src);

	std::unique_ptr<possumwood::Shader> shader(new possumwood::Shader(GL_FRAGMENT_SHADER));

	shader->compile(src);
	result = shader->state();

	if(!shader->state().errored())
		data.set(a_shader, std::shared_ptr<const possumwood::Shader>(shader.release()));
	else
		data.set(a_shader, std::shared_ptr<const possumwood::Shader>());

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source");
	meta.addAttribute(a_shader, "shader");

	meta.addInfluence(a_src, a_shader);

	meta.setCompute(&compute);
	meta.setEditor<Editor>();
}

possumwood::NodeImplementation s_impl("shaders/shader", init);

}
