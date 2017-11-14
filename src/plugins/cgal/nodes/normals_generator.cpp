#include <possumwood_sdk/node_implementation.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "datatypes/meshes.h"

#include <CGAL/Polygon_mesh_processing/compute_normal.h>

namespace {

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<Meshes> a_inMeshes;
dependency_graph::OutAttr<Meshes> a_outMesh;

// typename CGAL::Kernel_traits<
//   typename boost::property_traits<
//     typename boost::property_map<Mesh, CGAL::vertex_point_t>::type>::value_type>::Kernel Kernel;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Enum mode = data.get(a_mode);
	const Meshes& inMeshes = data.get(a_inMeshes);

	Meshes result;
	for(auto& inMesh : inMeshes) {
		std::unique_ptr<CGALPolyhedron> mesh(new CGALPolyhedron(inMesh.mesh()));

		// remove any existing normals
		{
			auto vertNormals = mesh->property_map<CGALPolyhedron::Vertex_index, possumwood::CGALKernel::Vector_3>("normals");
			if(vertNormals.second)
				mesh->remove_property_map(vertNormals.first);

			auto faceNormals = mesh->property_map<CGALPolyhedron::Face_index, possumwood::CGALKernel::Vector_3>("normals");
			if(faceNormals.second)
				mesh->remove_property_map(faceNormals.first);
		}

		// request for vertex normals
		if(mode.value() == "Per-vertex normals") {
			auto vertNormals = mesh->add_property_map<CGALPolyhedron::Vertex_index, possumwood::CGALKernel::Vector_3>("normals");
			CGAL::Polygon_mesh_processing::compute_vertex_normals(*mesh, vertNormals.first,
				CGAL::Polygon_mesh_processing::parameters::geom_traits(possumwood::CGALKernel()));
		}

		// request for half-edge (polygon-vertex) normals
		else if(mode.value() == "Per-face normals") {
			auto faceNormals = mesh->add_property_map<CGALPolyhedron::Face_index, possumwood::CGALKernel::Vector_3>("normals");
			CGAL::Polygon_mesh_processing::compute_face_normals(*mesh, faceNormals.first,
				CGAL::Polygon_mesh_processing::parameters::geom_traits(possumwood::CGALKernel()));
		}

		result.addMesh(inMesh.name(), std::move(mesh));
	}

	data.set(a_outMesh, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_mode, "mode",
	                  possumwood::Enum({"Per-face normals", "Per-vertex normals"}));
	meta.addAttribute(a_inMeshes, "input");
	meta.addAttribute(a_outMesh, "output");

	meta.addInfluence(a_mode, a_outMesh);
	meta.addInfluence(a_inMeshes, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/normals_generator", init);
}
