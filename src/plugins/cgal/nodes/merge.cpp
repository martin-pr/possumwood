#include <possumwood_sdk/node_implementation.h>

#include "datatypes/meshes.h"
#include "cgal.h"

namespace {

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

dependency_graph::InAttr<Meshes> a_inMesh1, a_inMesh2;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Meshes& input1 = data.get(a_inMesh1);
	const Meshes& input2 = data.get(a_inMesh2);

	Meshes output = input1;

	for(auto& mesh : input2)
		output.addMesh(mesh);

	data.set(a_outMesh, output);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh1, "in_mesh_1");
	meta.addAttribute(a_inMesh2, "in_mesh_2");
	meta.addAttribute(a_outMesh, "out_mesh");

	meta.addInfluence(a_inMesh1, a_outMesh);
	meta.addInfluence(a_inMesh2, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/merge", init);
}
