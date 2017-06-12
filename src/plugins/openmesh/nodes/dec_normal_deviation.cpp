#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <OpenMesh/Tools/Decimater/ModNormalDeviationT.hh>

#include "datatypes/decimater_module.h"

#include "io/decimater_module.h"
#include "io/mesh.h"

#include "openmesh.h"

namespace {

dependency_graph::InAttr<std::vector<DecimaterModule>> a_in;
dependency_graph::InAttr<bool> a_binary;
dependency_graph::InAttr<float> a_maxDeviation;
dependency_graph::OutAttr<std::vector<DecimaterModule>> a_out;

void compute(dependency_graph::Values& data) {
	std::vector<DecimaterModule> decs = data.get(a_in);
	bool binary = data.get(a_binary);
	float maxDeviation = data.get(a_maxDeviation);

	decs.push_back(DecimaterModule([binary, maxDeviation](OpenMesh::Decimater::DecimaterT<Mesh>& dec) {
		if(!dec.mesh().has_face_normals())
			dec.mesh().request_face_normals();

		OpenMesh::Decimater::ModNormalDeviationT<Mesh>::Handle mod;
		dec.add(mod);
		dec.module(mod).set_binary(binary);
		dec.module(mod).set_normal_deviation(maxDeviation);
	}));

	data.set(a_out, decs);
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_modules", std::vector<DecimaterModule>());
	meta.addAttribute(a_binary, "binary", true);
	meta.addAttribute(a_maxDeviation, "max_deviation", 15.0f);
	meta.addAttribute(a_out, "out_modules", std::vector<DecimaterModule>());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_binary, a_out);
	meta.addInfluence(a_maxDeviation, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("openmesh/decimater/module_normal_deviation", init);

}
