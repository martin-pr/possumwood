#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>

#include "datatypes/decimater_module.h"

#include "io/decimater_module.h"
#include "io/mesh.h"

#include "openmesh.h"
#include "om_log.h"

namespace {

dependency_graph::InAttr<std::vector<DecimaterModule>> a_in;
dependency_graph::InAttr<float> a_maxNormalDev, a_errorFactor;
dependency_graph::OutAttr<std::vector<DecimaterModule>> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	OMLog logRedirect;

	std::vector<DecimaterModule> decs = data.get(a_in);
	const float maxNormalDev = data.get(a_maxNormalDev);
	const float errorFactor = data.get(a_errorFactor);

	decs.push_back(DecimaterModule([maxNormalDev, errorFactor](OpenMesh::Decimater::DecimaterT<Mesh>& dec) {
		if(!dec.mesh().has_face_normals()) {
			dec.mesh().request_face_normals();
			dec.mesh().update_normals();
		}

		OpenMesh::Decimater::ModNormalFlippingT<Mesh>::Handle mod;
		dec.add(mod);
		dec.module(mod).set_max_normal_deviation(maxNormalDev);
		dec.module(mod).set_error_tolerance_factor(errorFactor);
	}));

	data.set(a_out, decs);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_modules", std::vector<DecimaterModule>());
	meta.addAttribute(a_maxNormalDev, "max_normal_deviation", 0.2f);
	meta.addAttribute(a_errorFactor, "error_tolerance_factor", 0.1f);
	meta.addAttribute(a_out, "out_modules", std::vector<DecimaterModule>());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_maxNormalDev, a_out);
	meta.addInfluence(a_errorFactor, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("openmesh/decimater/module_normal_flipping", init);

}
