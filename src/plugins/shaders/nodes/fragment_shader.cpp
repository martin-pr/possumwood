#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/shader.h"
#include "ui/shader_editor.h"

namespace {

dependency_graph::InAttr<std::string> a_src;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::FragmentShader>> a_shader;

class Editor : public possumwood::ShaderEditor {
	public:
		Editor() : ShaderEditor(a_src) {
		}

};

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const std::string& src = data.get(a_src);

	std::unique_ptr<possumwood::FragmentShader> shader(new possumwood::FragmentShader());

	shader->compile(src);
	result = shader->state();

	if(!shader->state().errored())
		data.set(a_shader, std::shared_ptr<const possumwood::FragmentShader>(shader.release()));
	else
		data.set(a_shader, std::shared_ptr<const possumwood::FragmentShader>());

	return result;
}

static const std::string defaultSrc =
	"#version 330\n"
	"\n"
	"// generic attributes\n"
	"uniform mat4 iProjection;  // projection matrix\n"
	"uniform mat4 iModelView;   // modelview matrix\n"
	"uniform vec2 iResolution;  // viewport resolution\n"
	"uniform float iTime;       // current time from the timeline\n"
	"\n"
	"// attributes useable for raytracing\n"
	"in vec3 iNearPosition;     // position of fragment-corresponding point on near plane\n"
	"in vec3 iFarPosition;      // position of fragment-corresponding point on far plane\n"
	"\n"
	"// output colour\n"
	"layout(location=0) out vec4 color;\n"
	"\n"
	"// a simple integer-based checkerboard pattern\n"
	"float tile(vec2 pos) {\n"
	"	return (\n"
	"		int(pos.x + max(0.0, sign(pos.x))) + \n"
	"		int(pos.y + max(0.0, sign(pos.y)))\n"
	"	) % 2;\n"
	"}\n"
	"\n"
	"// computes Z-buffer depth value, and converts the range.\n"
	"// ref: https://stackoverflow.com/questions/10264949/glsl-gl-fragcoord-z-calculation-and-setting-gl-fragdepth\n"
	"float computeDepth(vec3 pos) {\n"
	"	// get the clip-space coordinates\n"
	"	vec4 eye_space_pos = iModelView * vec4(pos.xyz, 1.0);\n"
	"	vec4 clip_space_pos = iProjection * eye_space_pos;\n"
	"\n"
	"	// get the depth value in normalized device coordinates\n"
	"	float ndc_depth = clip_space_pos.z / clip_space_pos.w;\n"
	"\n"
	"	// and compute the range based on gl_DepthRange settings (usually not necessary, but still)\n"
	"	float far = gl_DepthRange.far; \n"
	"	float near = gl_DepthRange.near;\n"
	"\n"
	"	float depth = (((far-near) * ndc_depth) + near + far) / 2.0;\n"
	"\n"
	"	// and return the result\n"
	"	return depth;\n"
	"}\n"
	"\n"
	"void main() {\n"
	"	// find the t parameter where Y = 0 (intersection with ground plane)\n"
	"	float t = iNearPosition.y / (iNearPosition.y - iFarPosition.y);\n"
	"\n"
	"	// not intersecting with ground plane at all - discard\n"
	"	if(t < 0.0)\n"
	"		discard;\n"
	"\n"
	"	// find the intersecting position\n"
	"	vec3 pos = iNearPosition + t * (iFarPosition - iNearPosition);\n"
	"\n"
	"	// and make the checkerboard pattern\n"
	"	float col = tile(pos.xz) * 0.3;\n"
	"	col = col + tile(pos.xz / 10.0) * 0.15;\n"
	"	col = col + tile(pos.xz / 100.0) * 0.075;\n"
	"\n"
	"	// simple attenuation with distance\n"
	"	float dist = sqrt(pos.x*pos.x + pos.z*pos.z);\n"
	"	float dim = 1.0 - dist / (dist + 100.0);\n"
	"	col = col * dim;\n"
	"\n"
	"	// output colour\n"
	"	color = vec4(col, col, col, 1);\n"
	"\n"
	"	// convert the world-space position to a depth value, to keep Z buffer working\n"
	"	gl_FragDepth = computeDepth(pos);\n"
	"}\n";


void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source", defaultSrc);
	meta.addAttribute(a_shader, "shader");

	meta.addInfluence(a_src, a_shader);

	meta.setCompute(&compute);
	meta.setEditor<Editor>();
}

possumwood::NodeImplementation s_impl("shaders/fragment_shader", init);

}
