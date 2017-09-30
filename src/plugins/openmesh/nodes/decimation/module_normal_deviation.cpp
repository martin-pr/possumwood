#include <possumwood_sdk/node_implementation.h>

#include <OpenMesh/Tools/Decimater/ModNormalDeviationT.hh>

#include "datatypes/decimater_module.h"

#include "openmesh.h"
#include "om_log.h"

namespace {

dependency_graph::InAttr<std::vector<DecimaterModule>> a_in;
dependency_graph::InAttr<bool> a_binary;
dependency_graph::InAttr<float> a_maxDeviation, a_errorFactor;
dependency_graph::OutAttr<std::vector<DecimaterModule>> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	OMLog logRedirect;

	std::vector<DecimaterModule> decs = data.get(a_in);
	bool binary = data.get(a_binary);
	float maxDeviation = data.get(a_maxDeviation);
	float errorFactor = data.get(a_errorFactor);

	decs.push_back(DecimaterModule([binary, maxDeviation, errorFactor](OpenMesh::Decimater::DecimaterT<Mesh>& dec) {
		if(!dec.mesh().has_face_normals()) {
			dec.mesh().request_face_normals();
			dec.mesh().update_normals();
		}

		OpenMesh::Decimater::ModNormalDeviationT<Mesh>::Handle mod;
		dec.add(mod);
		dec.module(mod).set_binary(binary);
		dec.module(mod).set_normal_deviation(maxDeviation);
		dec.module(mod).set_error_tolerance_factor(errorFactor);
	}));

	data.set(a_out, decs);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_modules", std::vector<DecimaterModule>());
	meta.addAttribute(a_binary, "binary", false);
	meta.addAttribute(a_maxDeviation, "max_deviation", 300.0f);
	meta.addAttribute(a_errorFactor, "error_tolerance_factor", 0.5f);
	meta.addAttribute(a_out, "out_modules", std::vector<DecimaterModule>());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_binary, a_out);
	meta.addInfluence(a_maxDeviation, a_out);
	meta.addInfluence(a_errorFactor, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("openmesh/decimation/module_normal_deviation", init);

}
