#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathVec.h>

#include "openmesh/datatypes/mesh.h"

#include "datatypes/vertex_data.inl"

namespace {

dependency_graph::OutAttr<std::shared_ptr<const possumwood::VertexData>> a_vd;
dependency_graph::InAttr<std::shared_ptr<const Mesh>> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	std::shared_ptr<const Mesh> mesh = data.get(a_mesh);

	if(!mesh)
		throw std::runtime_error("No mesh provided.");

	// we're drawing triangles
	std::unique_ptr<possumwood::VertexData> vd(new possumwood::VertexData(GL_TRIANGLES));

	// first, figure out how many triangles we have
	//   there has to be a better way to do this, come on
	std::size_t triangleCount = 0;
	for(auto f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it) {
		auto it = mesh->cfh_iter(*f_it);

		++it;
		++it;

		while(it.is_valid()) {
			++it;
			++triangleCount;
		}
	}

	// and build the buffers
	vd->addVBO<Imath::V3f>(
		"position",
		triangleCount * 3,
		possumwood::VertexData::kStatic,
		[mesh](Imath::V3f* iter, Imath::V3f* end) {
			// iterate over faces
			for(auto f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it) {
			    // get the face's half-edge iterator
			    auto he_it = mesh->cfh_iter(*f_it);

			    // remember first two vertices of the face
			    const auto& pt1h = mesh->to_vertex_handle(*he_it);
			    const auto& pt1 = mesh->point(pt1h);
			    ++he_it;

			    const auto& pt2h = mesh->to_vertex_handle(*he_it);
			    const auto& pt2 = mesh->point(pt2h);
			    ++he_it;

			    // iterate over remaining halfedges on that face
			    for(; he_it.is_valid(); ++he_it) {
			        *(iter++) = Imath::V3f(pt1[0],pt1[1],pt1[2]);
			        *(iter++) = Imath::V3f(pt2[0],pt2[1],pt2[2]);

			        const auto& pth = mesh->to_vertex_handle(*he_it);
			        const auto& pt = mesh->point(pth);
			        *(iter++) = Imath::V3f(pt[0],pt[1],pt[2]);
				}
			}

			assert(iter == end);
		}
		);

	data.set(a_vd, std::shared_ptr<const possumwood::VertexData>(vd.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_vd, "vertex_data");
	meta.addAttribute(a_mesh, "mesh");

	meta.addInfluence(a_mesh, a_vd);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("shaders/vertex_data/openmesh", init);

}
