#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/program.h"
#include "ui/shader_editor.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::VertexShader>> a_vs;
dependency_graph::InAttr<std::shared_ptr<const possumwood::GeometryShader>> a_gs;
dependency_graph::InAttr<std::shared_ptr<const possumwood::FragmentShader>> a_fs;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::Program>> a_program;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	std::shared_ptr<const possumwood::VertexShader> vs = data.get(a_vs);
	std::shared_ptr<const possumwood::GeometryShader> gs = data.get(a_gs);
	std::shared_ptr<const possumwood::FragmentShader> fs = data.get(a_fs);

	if(vs == nullptr || fs == nullptr) {
		result.addError("Both vertex and fragment shader are required for program linking!");
		data.set(a_program, std::shared_ptr<const possumwood::Program>());
	}

	else {
		std::unique_ptr<possumwood::Program> program(new possumwood::Program());

		program->addShader(*vs);
		program->addShader(*fs);
		if(gs)
			program->addShader(*gs);

		program->link();
		result = program->state();

		if(!program->state().errored())
			data.set(a_program, std::shared_ptr<const possumwood::Program>(program.release()));
		else
			data.set(a_program, std::shared_ptr<const possumwood::Program>());
	}

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_vs, "vertex_shader");
	meta.addAttribute(a_gs, "geometry_shader");
	meta.addAttribute(a_fs, "fragment_shader");

	meta.addAttribute(a_program, "program");

	meta.addInfluence(a_vs, a_program);
	meta.addInfluence(a_gs, a_program);
	meta.addInfluence(a_fs, a_program);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/program", init);

}
