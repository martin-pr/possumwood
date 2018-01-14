#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "datatypes/meshes.h"
#include "maths/io/vec3.h"
#include "builder.h"

namespace {

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

dependency_graph::InAttr<Imath::V3f> a_v0, a_v1, a_v2, a_v3;
dependency_graph::OutAttr<Meshes> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::vector<Imath::V3f> vertices;
	vertices.push_back(data.get(a_v0));
	vertices.push_back(data.get(a_v1));
	vertices.push_back(data.get(a_v2));
	vertices.push_back(data.get(a_v3));

	std::vector<std::array<std::size_t, 3>> faces;
	faces.push_back(std::array<std::size_t, 3>{{0, 1, 2}});
	faces.push_back(std::array<std::size_t, 3>{{1, 3, 2}});
	faces.push_back(std::array<std::size_t, 3>{{0, 3, 1}});
	faces.push_back(std::array<std::size_t, 3>{{3, 0, 2}});

	Meshes result;
	auto& mesh = result.addMesh("tetrahedron");

	{
		possumwood::CGALBuilder<possumwood::CGALPolyhedron::HalfedgeDS, typeof(vertices), typeof(faces)> builder(vertices, faces);
		mesh.polyhedron().delegate(builder);
	}

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
