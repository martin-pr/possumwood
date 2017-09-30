#include <possumwood_sdk/node_implementation.h>

#include <OpenMesh/Tools/Decimater/ModEdgeLengthT.hh>

#include "datatypes/decimater_module.h"

#include "openmesh.h"
#include "om_log.h"

namespace {

dependency_graph::InAttr<std::vector<DecimaterModule>> a_in;
dependency_graph::InAttr<bool> a_binary;
dependency_graph::InAttr<float> a_edgeLength, a_errorFactor;
dependency_graph::OutAttr<std::vector<DecimaterModule>> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	OMLog logRedirect;

	std::vector<DecimaterModule> decs = data.get(a_in);
	const bool binary = data.get(a_binary);
	const float edgeLength = data.get(a_edgeLength);
	const float errorFactor = data.get(a_errorFactor);

	decs.push_back(DecimaterModule([binary, edgeLength, errorFactor](OpenMesh::Decimater::DecimaterT<Mesh>& dec) {
		OpenMesh::Decimater::ModEdgeLengthT<Mesh>::Handle mod;
		dec.add(mod);
		dec.module(mod).set_binary(binary);
		dec.module(mod).set_edge_length(edgeLength);
		dec.module(mod).set_error_tolerance_factor(errorFactor);
	}));

	data.set(a_out, decs);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_modules", std::vector<DecimaterModule>());
	meta.addAttribute(a_binary, "binary", false);
	meta.addAttribute(a_edgeLength, "edge_length", 0.2f);
	meta.addAttribute(a_errorFactor, "error_tolerance_factor", 0.0f);
	meta.addAttribute(a_out, "out_modules", std::vector<DecimaterModule>());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_binary, a_out);
	meta.addInfluence(a_edgeLength, a_out);
	meta.addInfluence(a_errorFactor, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("openmesh/decimation/module_edge_length", init);

}
