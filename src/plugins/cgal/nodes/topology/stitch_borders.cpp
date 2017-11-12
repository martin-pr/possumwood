#include <possumwood_sdk/node_implementation.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "datatypes/polyhedron.h"

#include <CGAL/Polygon_mesh_processing/stitch_borders.h>

namespace {

typedef possumwood::CGALPolyhedron Mesh;

dependency_graph::InAttr<std::shared_ptr<const Mesh>> a_inMesh;
dependency_graph::OutAttr<std::shared_ptr<const Mesh>> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	const std::shared_ptr<const Mesh> inMesh = data.get(a_inMesh);

	if(inMesh != nullptr) {
		std::unique_ptr<Mesh> mesh(new Mesh(*inMesh));

		CGAL::Polygon_mesh_processing::stitch_borders(*mesh);

		data.set(a_outMesh, std::shared_ptr<const Mesh>(mesh.release()));
	}
	else
		data.set(a_outMesh, std::shared_ptr<const Mesh>());

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
