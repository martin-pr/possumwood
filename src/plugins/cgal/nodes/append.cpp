#include <possumwood_sdk/node_implementation.h>

#include "datatypes/meshes.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

typedef possumwood::CGALPolyhedron Mesh;

dependency_graph::InAttr<Meshes> a_inMesh1, a_inMesh2;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	Meshes result;

	for(auto& m : data.get(a_inMesh1))
		result.addMesh(m);

	for(auto& m : data.get(a_inMesh2))
		result.addMesh(m);

	data.set(a_outMesh, result);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh1, "in_mesh_1", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inMesh2, "in_mesh_2", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outMesh, "out_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inMesh1, a_outMesh);
	meta.addInfluence(a_inMesh2, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/append", init);
}  // namespace
