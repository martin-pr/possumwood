#include <possumwood_sdk/node_implementation.h>

#include "cgal.h"
#include "datatypes/meshes.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

dependency_graph::InAttr<Meshes> a_inMesh;
dependency_graph::InAttr<std::string> a_name;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	Meshes result = data.get(a_inMesh);
	for(auto& mesh_ : result) {
		auto& mesh = mesh_.edit();

		if(mesh.faceProperties().hasProperty(data.get(a_name)))
			mesh.faceProperties().removeProperty(data.get(a_name));

		if(mesh.halfedgeProperties().hasProperty(data.get(a_name)))
			mesh.halfedgeProperties().removeProperty(data.get(a_name));

		if(mesh.vertexProperties().hasProperty(data.get(a_name)))
			mesh.vertexProperties().removeProperty(data.get(a_name));
	}

	data.set(a_outMesh, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "in_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_name, "name", std::string("N"));
	meta.addAttribute(a_outMesh, "out_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_name, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/remove_property", init);
}  // namespace
