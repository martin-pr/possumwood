#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "datatypes/mesh.h"
#include "openmesh.h"
#include "om_log.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<std::shared_ptr<const Mesh>> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	OMLog logRedirect;

	const possumwood::Filename filename = data.get(a_filename);

	OpenMesh::IO::Options ropt;

	std::unique_ptr<Mesh> mesh(new Mesh());
	bool result = OpenMesh::IO::read_mesh(*mesh, filename.filename().string(), ropt);

	if(!result)
		logRedirect.state().addError("Error loading '" + filename.filename().string() + "'");
	else
		logRedirect.state().addInfo("Loaded '" + filename.filename().string() + "'");

	data.set(a_mesh, std::shared_ptr<const Mesh>(mesh.release()));

	return logRedirect.state();
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
