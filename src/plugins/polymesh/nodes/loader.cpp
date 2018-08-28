#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <fstream>

#include "generic_polymesh.inl"
#include "traits.h"
#include "obj.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<possumwood::polymesh::GenericPolymesh> a_polymesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	if(filename.filename().extension() == ".obj") {
		data.set(a_polymesh, possumwood::polymesh::loadObj(filename.filename()));
	}
	else
		throw std::runtime_error("Unknown extension " + filename.filename().extension().string());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"OBJ files (*.obj)",
	}));
	meta.addAttribute(a_polymesh, "generic_polymesh");

	meta.addInfluence(a_filename, a_polymesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("polymesh/loader", init);
}
