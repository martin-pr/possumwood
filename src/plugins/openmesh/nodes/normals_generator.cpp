#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include "possumwood_sdk/datatypes/enum.h"

#include "io/mesh.h"
#include "openmesh.h"

namespace {

dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<std::shared_ptr<const Mesh>> a_inMesh;
dependency_graph::OutAttr<std::shared_ptr<const Mesh>> a_outMesh;

void compute(dependency_graph::Values& data) {
	const possumwood::Enum mode = data.get(a_mode);
	const std::shared_ptr<const Mesh> inMesh = data.get(a_inMesh);

	if(inMesh != nullptr) {
		std::unique_ptr<Mesh> mesh(new Mesh(*inMesh));

		// face normals serve as normals source for all other types - always compute
		//   first, to allow the generation algorithm run correctly
		mesh->request_face_normals();
		mesh->update_normals();

		// request for vertex normals
		if(mode.value() == "Per-vertex normals") {
			mesh->request_vertex_normals();
			mesh->update_normals();
		}

		// request for half-edge (polygon-vertex) normals
		else if(mode.value() == "Per-halfedge normals") {
			mesh->request_halfedge_normals();
			mesh->update_normals();
		}

		data.set(a_outMesh, std::shared_ptr<const Mesh>(mesh.release()));
	}
	else
		data.set(a_outMesh, std::shared_ptr<const Mesh>());
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_mode, "mode", possumwood::Enum({
		"Per-face normals",
		"Per-vertex normals",
		"Per-halfedge normals"
	}));
	meta.addAttribute(a_inMesh, "input");
	meta.addAttribute(a_outMesh, "output");

	meta.addInfluence(a_mode, a_outMesh);
	meta.addInfluence(a_inMesh, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("openmesh/normals_generator", init);

}
