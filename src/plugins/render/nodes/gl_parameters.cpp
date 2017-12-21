#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/gl_parameters.h"

namespace {

dependency_graph::InAttr<float> a_lineWidth;
dependency_graph::OutAttr<possumwood::GLParameters> a_params;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	possumwood::GLParameters params;
	params.setLineWidth(data.get(a_lineWidth));

	data.set(a_params, params);

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_lineWidth, "line_width", 1.0f);
	meta.addAttribute(a_params, "gl_params");

	meta.addInfluence(a_lineWidth, a_params);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/gl_parameters", init);

}
