#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include "datatypes/meshes.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

typedef possumwood::CGALPolyhedron Mesh;

dependency_graph::InAttr<Meshes> a_inMesh;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	Meshes result = data.get(a_inMesh);

	for(auto& mesh : result)
		CGAL::Polygon_mesh_processing::triangulate_faces(mesh.edit().polyhedron());

	data.set(a_outMesh, result);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "input", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outMesh, "output", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inMesh, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/topology/triangulate_faces", init);
}  // namespace
