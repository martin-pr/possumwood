#include <CGAL/convex_hull_3.h>

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

	// inefficient - an adaptor to replace this would be much better
	std::vector<possumwood::CGALKernel::Point_3> points;
	for(auto& mesh : data.get(a_inMesh))
		points.insert(points.end(), mesh.polyhedron().points_begin(), mesh.polyhedron().points_end());

	possumwood::Mesh mesh("convex_hull");
	CGAL::convex_hull_3(points.begin(), points.end(), mesh.edit().polyhedron());

	Meshes result;
	result.addMesh(mesh);
	data.set(a_outMesh, result);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "in_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outMesh, "out_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inMesh, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/topology/convex_hull", init);
}  // namespace
