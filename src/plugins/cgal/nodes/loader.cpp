#include <fstream>
#include <sstream>

#include <CGAL/IO/OBJ_reader.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>

#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include "builder.h"
#include "datatypes/meshes.h"
#include "errors.h"
#include "meshes.h"
#include "obj.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::InAttr<std::string> a_name;
dependency_graph::OutAttr<possumwood::Meshes> a_polyhedron;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	const possumwood::Filename filename = data.get(a_filename);

	std::vector<possumwood::CGALKernel::Point_3> points;
	std::vector<std::vector<std::size_t>> faces;

	possumwood::Meshes meshes;
	meshes.addMesh(possumwood::loadObj(filename.filename(), data.get(a_name)));

	data.set(a_polyhedron, meshes);

	return redirect.state();
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
