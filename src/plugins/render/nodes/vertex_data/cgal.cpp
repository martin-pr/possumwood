#include "cgal/datatypes/meshes.h"

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathVec.h>

#include "datatypes/vertex_data.inl"

namespace {

template<std::size_t WIDTH, typename T>
void assignArray(std::array<float, WIDTH>& arr, const T& val) {
	for(std::size_t a=0;a<WIDTH;++a)
		arr[a] = val[a];
}

template <std::size_t WIDTH, typename T>
void addVBO(possumwood::VertexData& vd, const std::string& name, const std::string& propertyName,
            std::size_t triangleCount,
            const possumwood::Meshes& mesh) {

	vd.addVBO<std::array<float, WIDTH>>(
	    name, triangleCount * WIDTH, possumwood::VertexData::kStatic,
	    [mesh, propertyName](std::array<float, WIDTH>* iter, std::array<float, WIDTH>* end) {

		    std::size_t ctr = 0;

		    // iterate over faces
		    for(auto& m : mesh) {
			    auto prop =
			        m.mesh()
			            .property_map<possumwood::CGALPolyhedron::Vertex_index,
			                          T>(propertyName.c_str());

			    for(auto it = m.mesh().faces_begin(); it != m.mesh().faces_end(); ++it) {
				    auto vertices = m.mesh().vertices_around_face(m.mesh().halfedge(*it));

				    if(vertices.size() >= 2) {
					    auto it = vertices.begin();

					    auto& val1 = prop.first[*it];
					    ++it;

					    auto& val2 = prop.first[*it];
					    ++it;

					    while(it != vertices.end()) {
						    auto& val = prop.first[*it];

						    assignArray(*(iter++), val1);
						    assignArray(*(iter++), val2);
						    assignArray(*(iter++), val);

						    ++it;

						    ++ctr;
					    }
				    }
			    }
			}

		    assert(iter == end);
		});
}

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

dependency_graph::OutAttr<std::shared_ptr<const possumwood::VertexData>> a_vd;
dependency_graph::InAttr<Meshes> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const Meshes mesh = data.get(a_mesh);

	// we're drawing triangles
	std::unique_ptr<possumwood::VertexData> vd(
	    new possumwood::VertexData(GL_TRIANGLES));

	// first, figure out how many triangles we have
	//   there has to be a better way to do this, come on
	std::size_t triangleCount = 0;
	for(auto& m : mesh)
		for(auto it = m.mesh().faces_begin(); it != m.mesh().faces_end();
		    ++it) {
			auto vertices =
			    m.mesh().vertices_around_face(m.mesh().halfedge(*it));

			if(vertices.size() > 2)
				triangleCount += (vertices.size() - 2);
		}

	// and build the buffers
	if(triangleCount > 0) {
		addVBO<3, possumwood::CGALKernel::Point_3>(*vd, "position", "v:point", triangleCount, mesh);

		unsigned normalPropMapCounter = 0;
		for(auto& m : mesh) {
			auto vertPropList =
			    m.mesh().properties<possumwood::CGALPolyhedron::Vertex_index>();
			if(std::find(vertPropList.begin(), vertPropList.end(), "normals") !=
			   vertPropList.end())
				++normalPropMapCounter;
			else {
				auto facePropList =
				    m.mesh().properties<possumwood::CGALPolyhedron::Face_index>();
				if(std::find(facePropList.begin(), facePropList.end(), "normals") !=
				   facePropList.end())
					++normalPropMapCounter;
			}
		}

		if(normalPropMapCounter != 0 && normalPropMapCounter != mesh.size())
			result.addWarning(
			    "Inconsistent normals between meshes - normals are ignored when "
			    "building the VBO.");

		if(normalPropMapCounter == mesh.size()) {
			vd->addVBO<std::array<float, 3>>(
			    "normal", triangleCount * 3, possumwood::VertexData::kStatic,
			    [mesh](std::array<float, 3>* iter, std::array<float, 3>* end) {

				    for(auto& m : mesh) {
					    auto vertNormals =
					        m.mesh()
					            .property_map<
					                possumwood::CGALPolyhedron::Vertex_index,
					                possumwood::CGALKernel::Vector_3>("normals");
					    auto faceNormals =
					        m.mesh()
					            .property_map<
					                possumwood::CGALPolyhedron::Face_index,
					                possumwood::CGALKernel::Vector_3>("normals");

					    if(vertNormals.second) {
						    for(auto fit = m.mesh().faces_begin();
						        fit != m.mesh().faces_end(); ++fit) {
							    auto vertices = m.mesh().vertices_around_face(
							        m.mesh().halfedge(*fit));

							    if(vertices.size() >= 2) {
								    auto it = vertices.begin();

								    auto& n1 = vertNormals.first[*it];
								    ++it;

								    auto& n2 = vertNormals.first[*it];
								    ++it;

								    while(it != vertices.end()) {
									    auto& n = vertNormals.first[*it];

									    *(iter++) = std::array<float, 3>{{n1[0], n1[1], n1[2]}};
									    *(iter++) = std::array<float, 3>{{n2[0], n2[1], n2[2]}};
									    *(iter++) = std::array<float, 3>{{n[0], n[1], n[2]}};

									    ++it;
								    }
							    }
						    }
					    }
					    else if(faceNormals.second) {
						    // iterate over faces
						    for(auto fit = m.mesh().faces_begin();
						        fit != m.mesh().faces_end(); ++fit) {
							    auto vertices = m.mesh().vertices_around_face(
							        m.mesh().halfedge(*fit));

							    auto& n = faceNormals.first[*fit];

							    if(vertices.size() >= 2) {
								    auto it = vertices.begin();

								    ++it;
								    ++it;

								    while(it != vertices.end()) {
									    *(iter++) = std::array<float, 3>{{n[0], n[1], n[2]}};
									    *(iter++) = std::array<float, 3>{{n[0], n[1], n[2]}};
									    *(iter++) = std::array<float, 3>{{n[0], n[1], n[2]}};

									    ++it;
								    }
							    }
						    }
					    }
				    }

				    assert(iter == end);
				});
		}
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
