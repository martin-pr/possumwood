#include <possumwood_sdk/node_implementation.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "datatypes/meshes.h"

#include <CGAL/Polygon_mesh_processing/stitch_borders.h>

namespace {

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

typedef possumwood::CGALPolyhedron Mesh;

dependency_graph::InAttr<Meshes> a_inMesh;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	Meshes result = data.get(a_inMesh);

	for(auto& mesh : result)
		CGAL::Polygon_mesh_processing::stitch_borders(mesh.polyhedron());

	data.set(a_outMesh, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "input", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outMesh, "output", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inMesh, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/topology/stitch_borders", init);
}
