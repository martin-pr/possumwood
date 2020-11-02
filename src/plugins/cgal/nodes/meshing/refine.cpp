#include <CGAL/Polygon_mesh_processing/refine.h>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/meshes.h"
#include "datatypes/selection.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::FaceSelection;
using possumwood::Meshes;

typedef possumwood::CGALPolyhedron Mesh;

dependency_graph::InAttr<Meshes> a_inMesh;
dependency_graph::InAttr<float> a_densityFactor;
dependency_graph::OutAttr<Meshes> a_outMesh;
dependency_graph::OutAttr<FaceSelection> a_selection;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	Meshes result;
	FaceSelection selection;
	for(auto mesh : data.get(a_inMesh)) {
		auto& editableMesh = mesh.edit();

		FaceSelection::Item new_facets(mesh);
		std::vector<possumwood::CGALPolyhedron::Vertex_handle> new_vertices;

		CGAL::Polygon_mesh_processing::refine(
		    editableMesh.polyhedron(), CGAL::faces(editableMesh.polyhedron()), std::back_inserter(new_facets),
		    std::back_inserter(new_vertices),
		    CGAL::Polygon_mesh_processing::parameters::density_control_factor(data.get(a_densityFactor)));

		result.addMesh(mesh);
		selection.push_back(new_facets);
	}

	data.set(a_outMesh, result);
	data.set(a_selection, selection);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "in_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_densityFactor, "density_factor", 2.0f);
	meta.addAttribute(a_outMesh, "out_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_selection, "selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_densityFactor, a_outMesh);
	meta.addInfluence(a_inMesh, a_selection);
	meta.addInfluence(a_densityFactor, a_selection);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/meshing/refine", init);
}  // namespace
