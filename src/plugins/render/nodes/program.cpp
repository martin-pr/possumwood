#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/program.h"

namespace {

dependency_graph::InAttr<possumwood::VertexShader> a_vs;
dependency_graph::InAttr<possumwood::GeometryShader> a_gs;
dependency_graph::InAttr<possumwood::FragmentShader> a_fs;
dependency_graph::OutAttr<possumwood::Program> a_program;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	possumwood::VertexShader vs = data.get(a_vs);
	possumwood::GeometryShader gs = data.get(a_gs);
	possumwood::FragmentShader fs = data.get(a_fs);

	vs.compile();
	fs.compile();
	fs.compile();

	if(vs.id() == 0 || fs.id() == 0) {
		result.addError("Both vertex and fragment shader are required for program linking!");
		data.set(a_program, possumwood::Program());
	}

	else {
		std::vector<possumwood::Shader> shaders;
		shaders.push_back(vs);
		shaders.push_back(fs);
		if(gs.id() != 0)
			shaders.push_back(gs);

		possumwood::Program program(shaders);

		program.link();
		result = program.state();

		if(!program.state().errored())
			data.set(a_program, program);
		else
			data.set(a_program, possumwood::Program());
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
