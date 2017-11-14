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
	const Meshes inMeshes = data.get(a_inMesh);

	Meshes result;

	for(auto& inMesh : inMeshes) {
		std::unique_ptr<Mesh> mesh(new Mesh(inMesh.mesh()));

		CGAL::Polygon_mesh_processing::stitch_borders(*mesh);

		result.addMesh(inMesh.name(), std::move(mesh));
	}

	data.set(a_outMesh, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "input");
	meta.addAttribute(a_outMesh, "output");

	meta.addInfluence(a_inMesh, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/topology/stitch_borders", init);
}
