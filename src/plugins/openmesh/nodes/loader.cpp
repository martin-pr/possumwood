#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include "datatypes/filename.h"

#include "io/filename.h"
#include "io/mesh.h"

#include "openmesh.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<std::shared_ptr<const Mesh>> a_mesh;

void compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	OpenMesh::IO::Options ropt;

	std::unique_ptr<Mesh> mesh(new Mesh());
	bool result = OpenMesh::IO::read_mesh(*mesh, filename.filename().string(), ropt);

	if(!result)
		std::cout << "Error loading " << filename.filename() << std::endl;
	else
		std::cout << "Loaded " << filename.filename() << std::endl;

	data.set(a_mesh, std::shared_ptr<const Mesh>(mesh.release()));
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"All supported files (*.obj *.off *.stl)",
		"OBJ files (*.obj)",
		"OFF files (*.pff)",
		"STL files (*.stl)",
	}));
	meta.addAttribute(a_mesh, "mesh");

	meta.addInfluence(a_filename, a_mesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("openmesh/loader", init);

}
