#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Utils/vector_traits.hh>
#include <OpenMesh/Core/Utils/vector_cast.hh>
#include <OpenMesh/Tools/Decimater/ModRoundnessT.hh>

#include "datatypes/decimater_module.h"

#include "io/decimater_module.h"
#include "io/mesh.h"

#include "openmesh.h"

namespace {

dependency_graph::InAttr<std::vector<DecimaterModule>> a_in;
dependency_graph::InAttr<bool> a_binary;
dependency_graph::InAttr<float> a_minRoundness, a_errorFactor;
dependency_graph::OutAttr<std::vector<DecimaterModule>> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::vector<DecimaterModule> decs = data.get(a_in);
	const bool binary = data.get(a_binary);
	const float minRoundness = data.get(a_minRoundness);
	const float errorFactor = data.get(a_errorFactor);

	decs.push_back(DecimaterModule([binary, minRoundness, errorFactor](OpenMesh::Decimater::DecimaterT<Mesh>& dec) {
		OpenMesh::Decimater::ModRoundnessT<Mesh>::Handle mod;
		dec.add(mod);
		dec.module(mod).set_min_roundness(minRoundness, binary);
		dec.module(mod).set_error_tolerance_factor(errorFactor);
	}));

	data.set(a_out, decs);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_modules", std::vector<DecimaterModule>());
	meta.addAttribute(a_binary, "binary", false);
	meta.addAttribute(a_minRoundness, "min_roundness", 0.2f);
	meta.addAttribute(a_errorFactor, "error_tolerance_factor", 0.0f);
	meta.addAttribute(a_out, "out_modules", std::vector<DecimaterModule>());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_binary, a_out);
	meta.addInfluence(a_minRoundness, a_out);
	meta.addInfluence(a_errorFactor, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("openmesh/decimater/module_roundness", init);

}
