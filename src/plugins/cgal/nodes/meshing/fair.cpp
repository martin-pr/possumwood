#include <Eigen/Core>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <CGAL/Polygon_mesh_processing/fair.h>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/meshes.h"
#include "errors.h"

// // hacking boost begin and end - fairing seems to be assuming a version of boost we don't have
// namespace boost {
// using boost::range::begin;
// using boost::range::end;
// }  // namespace boost

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

typedef possumwood::CGALPolyhedron Mesh;

dependency_graph::InAttr<Meshes> a_inMesh;
dependency_graph::InAttr<unsigned> a_continuity;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	Meshes result;
	for(auto mesh : data.get(a_inMesh)) {
		auto& editableMesh = mesh.edit();

		CGAL::Polygon_mesh_processing::fair(
		    editableMesh.polyhedron(), CGAL::vertices(editableMesh.polyhedron()),
		    CGAL::Polygon_mesh_processing::parameters::fairing_continuity(data.get(a_continuity)));

		result.addMesh(mesh);
	}

	data.set(a_outMesh, result);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "in_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_continuity, "continuity", 2u);
	meta.addAttribute(a_outMesh, "out_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_continuity, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/meshing/fair", init);
}  // namespace
