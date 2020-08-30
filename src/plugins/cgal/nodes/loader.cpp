#include <fstream>
#include <sstream>

#include <CGAL/IO/OBJ_reader.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>

#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include "builder.h"
#include "datatypes/meshes.h"
#include "meshes.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::InAttr<std::string> a_name;
dependency_graph::OutAttr<possumwood::Meshes> a_polyhedron;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	std::vector<possumwood::CGALKernel::Point_3> points;
	std::vector<std::vector<std::size_t>> faces;

	std::ifstream file(filename.filename().string().c_str());
	bool result = CGAL::read_OBJ(file, points, faces);

	if(!result)
		throw std::runtime_error("Error reading file '" + filename.filename().string() + "'");

	possumwood::Meshes meshes;
	possumwood::Mesh& mesh = meshes.addMesh(data.get(a_name));

	possumwood::CGALBuilder<possumwood::CGALPolyhedron::HalfedgeDS, typeof(points), typeof(faces)> builder(points,
	                                                                                                       faces);
	mesh.polyhedron().delegate(builder);

	data.set(a_polyhedron, meshes);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename",
	                  possumwood::Filename({
	                      "OBJ files (*.obj)",
	                  }));
	meta.addAttribute(a_name, "name", std::string("mesh"));
	meta.addAttribute(a_polyhedron, "polyhedron", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_name, a_polyhedron);
	meta.addInfluence(a_filename, a_polyhedron);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/loader", init);
}  // namespace
