#include <regex>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/skeleton.h"
#include "datatypes/skinned_mesh.h"
#include "datatypes/meshes.h"
#include "cgal.h"

namespace {

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>>
    a_inMeshes;
dependency_graph::OutAttr<Meshes> a_outMeshes;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::shared_ptr<const std::vector<anim::SkinnedMesh>> meshes =
	    data.get(a_inMeshes);

	Meshes result;

	if(meshes != nullptr) {
		for(auto& m : *meshes) {
			std::unique_ptr<possumwood::CGALPolyhedron> out(
			    new possumwood::CGALPolyhedron());

			for(auto& v : m.vertices()) {
				const possumwood::CGALKernel::Point_3 pt(v.pos().x, v.pos().y,
				                                         v.pos().z);
				out->add_vertex(pt);
			}

			for(auto& p : m.polygons())
				out->add_face(p);

			result.addMesh(m.name(), std::move(out));
		}
	}

	data.set(a_outMeshes, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMeshes, "in_meshes");
	meta.addAttribute(a_outMeshes, "out_cgal");

	meta.addInfluence(a_inMeshes, a_outMeshes);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/mesh/to_cgal", init);
}
