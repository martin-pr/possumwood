#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <OpenMesh/Tools/Decimater/ModHausdorffT.hh>

#include "datatypes/decimater_module.h"

#include "io/decimater_module.h"
#include "io/mesh.h"

#include "openmesh.h"

namespace {

dependency_graph::InAttr<std::vector<DecimaterModule>> a_in;
dependency_graph::InAttr<float> a_tolerance, a_errorFactor;
dependency_graph::OutAttr<std::vector<DecimaterModule>> a_out;

void compute(dependency_graph::Values& data) {
	std::vector<DecimaterModule> decs = data.get(a_in);
	const float tolerance = data.get(a_tolerance);
	const float errorFactor = data.get(a_errorFactor);

	decs.push_back(DecimaterModule([tolerance, errorFactor](OpenMesh::Decimater::DecimaterT<Mesh>& dec) {
		OpenMesh::Decimater::ModHausdorffT<Mesh>::Handle mod;
		dec.add(mod);
		dec.module(mod).set_binary(true);
		dec.module(mod).set_tolerance(tolerance);
		dec.module(mod).set_error_tolerance_factor(errorFactor);
	}));

	data.set(a_out, decs);
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_modules", std::vector<DecimaterModule>());
	meta.addAttribute(a_tolerance, "tolerance", 0.2f);
	meta.addAttribute(a_errorFactor, "error_tolerance_factor", 0.0f);
	meta.addAttribute(a_out, "out_modules", std::vector<DecimaterModule>());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_tolerance, a_out);
	meta.addInfluence(a_errorFactor, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("openmesh/decimater/module_hausdorff", init);

}
