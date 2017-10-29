#include "cgal/datatypes/polyhedron.h"

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathVec.h>

#include "datatypes/vertex_data.inl"

namespace {

dependency_graph::OutAttr<std::shared_ptr<const possumwood::VertexData>> a_vd;
dependency_graph::InAttr<std::shared_ptr<const possumwood::CGALPolyhedron>> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	std::shared_ptr<const possumwood::CGALPolyhedron> mesh = data.get(a_mesh);

	if(!mesh)
		throw std::runtime_error("No mesh provided.");

	// we're drawing triangles
	std::unique_ptr<possumwood::VertexData> vd(new possumwood::VertexData(GL_TRIANGLES));

	// first, figure out how many triangles we have
	//   there has to be a better way to do this, come on
	std::size_t triangleCount = 0;
	for(auto it = mesh->faces_begin(); it != mesh->faces_end(); ++it) {
		auto vertices = mesh->vertices_around_face(mesh->halfedge(*it));

		if(vertices.size() > 2)
			triangleCount += (vertices.size() - 2);
	}

	// and build the buffers

	vd->addVBO<Imath::V3f>(
		"position",
		triangleCount * 3,
		possumwood::VertexData::kStatic,
		[mesh](Imath::V3f* iter, Imath::V3f* end) {

			std::size_t ctr = 0;

			// iterate over faces
			for(auto it = mesh->faces_begin(); it != mesh->faces_end(); ++it) {
				auto vertices = mesh->vertices_around_face(mesh->halfedge(*it));

				if(vertices.size() >= 2) {
					auto it = vertices.begin();

					auto& p1 = mesh->point(*it);
					++it;

					auto& p2 = mesh->point(*it);
					++it;

					while(it != vertices.end()) {
						auto& p = mesh->point(*it);

				        *(iter++) = Imath::V3f(p1[0],p1[1],p1[2]);
				        *(iter++) = Imath::V3f(p2[0],p2[1],p2[2]);
				        *(iter++) = Imath::V3f(p[0],p[1],p[2]);

						++it;

						++ctr;
					}
				}
			}

			assert(iter == end);
		});

	auto vertNormals = mesh->property_map<possumwood::CGALPolyhedron::Vertex_index, possumwood::CGALKernel::Vector_3>("normals");
	auto faceNormals = mesh->property_map<possumwood::CGALPolyhedron::Face_index, possumwood::CGALKernel::Vector_3>("normals");

	if(vertNormals.second) {
		vd->addVBO<Imath::V3f>(
			"normal",
			triangleCount * 3,
			possumwood::VertexData::kStatic,
			[mesh, vertNormals](Imath::V3f* iter, Imath::V3f* end) {
				// iterate over faces
				for(auto fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit) {
					auto vertices = mesh->vertices_around_face(mesh->halfedge(*fit));

					if(vertices.size() >= 2) {
						auto it = vertices.begin();

						auto& n1 = vertNormals.first[*it];
						++it;

						auto& n2 = vertNormals.first[*it];
						++it;

						while(it != vertices.end()) {
							auto& n = vertNormals.first[*it];

					        *(iter++) = Imath::V3f(n1[0],n1[1],n1[2]);
					        *(iter++) = Imath::V3f(n2[0],n2[1],n2[2]);
					        *(iter++) = Imath::V3f(n[0],n[1],n[2]);

							++it;
						}
					}
				}

				assert(iter == end);
			});
	}

	else if(faceNormals.second) {
		vd->addVBO<Imath::V3f>(
			"normal",
			triangleCount * 3,
			possumwood::VertexData::kStatic,
			[mesh, faceNormals](Imath::V3f* iter, Imath::V3f* end) {
				// iterate over faces
				for(auto fit = mesh->faces_begin(); fit != mesh->faces_end(); ++fit) {
					auto vertices = mesh->vertices_around_face(mesh->halfedge(*fit));

					auto& n = faceNormals.first[*fit];

					if(vertices.size() >= 2) {
						auto it = vertices.begin();

						++it;
						++it;

						while(it != vertices.end()) {
					        *(iter++) = Imath::V3f(n[0],n[1],n[2]);
					        *(iter++) = Imath::V3f(n[0],n[1],n[2]);
					        *(iter++) = Imath::V3f(n[0],n[1],n[2]);

							++it;
						}
					}
				}

				assert(iter == end);
			});
	}

	data.set(a_vd, std::shared_ptr<const possumwood::VertexData>(vd.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_vd, "vertex_data");
	meta.addAttribute(a_mesh, "mesh");

	meta.addInfluence(a_mesh, a_vd);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/vertex_data/cgal", init);

}
