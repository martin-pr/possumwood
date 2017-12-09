#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "datatypes/meshes.h"
#include "maths/io/vec3.h"

namespace {

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

dependency_graph::InAttr<Imath::V3f> a_v0, a_v1, a_v2, a_v3;
dependency_graph::OutAttr<Meshes> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::unique_ptr<possumwood::CGALPolyhedron> polyhedron(new possumwood::CGALPolyhedron());

	const Imath::V3f& v0 = data.get(a_v0);
	const Imath::V3f& v1 = data.get(a_v1);
	const Imath::V3f& v2 = data.get(a_v2);
	const Imath::V3f& v3 = data.get(a_v3);

	auto u = polyhedron->add_vertex(possumwood::CGALKernel::Point_3(v0.x, v0.y, v0.z));
	auto v = polyhedron->add_vertex(possumwood::CGALKernel::Point_3(v1.x, v1.y, v1.z));
	auto w = polyhedron->add_vertex(possumwood::CGALKernel::Point_3(v2.x, v2.y, v2.z));
	auto x = polyhedron->add_vertex(possumwood::CGALKernel::Point_3(v3.x, v3.y, v3.z));

	polyhedron->add_face(u, v, w);
	polyhedron->add_face(u, x, v);
	polyhedron->add_face(v, x, w);
	polyhedron->add_face(w, x, u);

	Meshes result;
	result.addMesh("tetrahedron", std::move(polyhedron));

	data.set(a_mesh, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_v0, "v0", Imath::V3f(0, 0, 0));
	meta.addAttribute(a_v1, "v1", Imath::V3f(1, 0, 0));
	meta.addAttribute(a_v2, "v2", Imath::V3f(0, 1, 0));
	meta.addAttribute(a_v3, "v3", Imath::V3f(0, 0, 1));

	meta.addAttribute(a_mesh, "mesh");

	meta.addInfluence(a_v0, a_mesh);
	meta.addInfluence(a_v1, a_mesh);
	meta.addInfluence(a_v2, a_mesh);
	meta.addInfluence(a_v3, a_mesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/tetrahedron", init);
}
