#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include "builder.h"
#include "datatypes/meshes.h"
#include "maths/io/vec3.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

dependency_graph::InAttr<unsigned> a_xSubd, a_ySubd;
dependency_graph::InAttr<float> a_xSize, a_ySize;
dependency_graph::InAttr<Imath::V3f> a_origin;
dependency_graph::OutAttr<Meshes> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::vector<Imath::V3f> vertices;
	for(unsigned y = 0; y <= data.get(a_ySubd); ++y) {
		const float yf = data.get(a_origin).y +
		                 (float)y / (float)(data.get(a_ySubd)) * data.get(a_ySize);
		for(unsigned x = 0; x <= data.get(a_xSubd); ++x) {
			const float xf = data.get(a_origin).x +
			                 (float)x / (float)(data.get(a_xSubd)) * data.get(a_xSize);

			vertices.push_back(Imath::V3f(xf, yf, data.get(a_origin).z));
		}
	}

	std::vector<std::array<std::size_t, 4>> faces;
	for(unsigned y = 0; y < data.get(a_ySubd); ++y)
		for(unsigned x = 0; x < data.get(a_xSubd); ++x) {
			const std::array<std::size_t, 4> arr{
			    {x + y * (data.get(a_xSubd) + 1), (x + 1) + y * (data.get(a_xSubd) + 1),
			     (x + 1) + (y + 1) * (data.get(a_xSubd) + 1),
			     x + (y + 1) * (data.get(a_xSubd) + 1)}};

			faces.push_back(arr);
		}

	Meshes result;
	auto& mesh = result.addMesh("grid");

	{
		possumwood::CGALBuilder<possumwood::CGALPolyhedron::HalfedgeDS, typeof(vertices),
		                        typeof(faces)>
		    builder(vertices, faces);
		mesh.polyhedron().delegate(builder);
	}

	data.set(a_mesh, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_xSubd, "subdivision/x", 1u);
	meta.addAttribute(a_ySubd, "subdivision/y", 1u);
	meta.addAttribute(a_xSize, "size/x", 1.0f);
	meta.addAttribute(a_ySize, "size/y", 1.0f);
	meta.addAttribute(a_origin, "origin", Imath::V3f(0, 0, 0));
	meta.addAttribute(a_mesh, "mesh");

	meta.addInfluence(a_xSubd, a_mesh);
	meta.addInfluence(a_ySubd, a_mesh);
	meta.addInfluence(a_xSize, a_mesh);
	meta.addInfluence(a_ySize, a_mesh);
	meta.addInfluence(a_origin, a_mesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/grid", init);
}  // namespace
