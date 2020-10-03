#include <GL/glew.h>
#include <GL/glu.h>
#include <ImathVec.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/node_implementation.h>

#include <boost/algorithm/string/join.hpp>

#include "cgal/datatypes/meshes.h"
#include "datatypes/vertex_data.inl"

namespace possumwood {

template <typename T>
struct VBOTraits<CGAL::Point_3<CGAL::Simple_cartesian<T>>> {
	typedef T element;
	static constexpr std::size_t width() {
		return 3;
	};
};

template <typename T>
struct VBOTraits<std::vector<T>> {
	typedef T element;
	static constexpr std::size_t width() {
		return 1;
	};
};
}  // namespace possumwood

namespace {

void addVerticesVBO(possumwood::VertexData& vd,
                    const std::string& name,
                    const possumwood::Meshes& meshes,
                    std::size_t triangleCount) {
	vd.addVBO<possumwood::CGALPolyhedron::Point_3>(
	    name, triangleCount * 3, possumwood::VertexData::kStatic,
	    [meshes, triangleCount](
	        possumwood::Buffer<typename possumwood::VBOTraits<possumwood::CGALPolyhedron::Point_3>::element>& buffer,
	        const possumwood::ViewportState& vs) {
		    // TODO: use index buffer instead
		    std::size_t index = 0;

		    for(const auto& m : meshes) {
			    for(auto fit = m.polyhedron().facets_begin(); fit != m.polyhedron().facets_end(); ++fit)
				    if(fit->facet_degree() > 2) {
					    auto vit = fit->facet_begin();

					    const auto& p = vit->vertex()->point();
					    ++vit;

					    for(std::size_t v = 2; v < fit->facet_degree(); ++v) {
						    const auto& p1 = vit->vertex()->point();
						    ++vit;
						    const auto& p2 = vit->vertex()->point();

						    buffer.element(index++) = p;
						    buffer.element(index++) = p1;
						    buffer.element(index++) = p2;
					    }
				    }
		    }

		    assert(index == triangleCount * 3);
	    });
}

template <typename T>
const T& identity(const T& val) {
	return val;
}

template <typename T, typename EXTRACT = std::function<T(const T&)>>
void addPerPointVBO(possumwood::VertexData& vd,
                    const std::string& name,
                    const std::string& propertyName,
                    std::size_t triangleCount,
                    const possumwood::Meshes& mesh,
                    EXTRACT extract = identity<T>) {
	vd.addVBO<T>(name, triangleCount * 3, possumwood::VertexData::kStatic,
	             [mesh, propertyName, extract](possumwood::Buffer<typename possumwood::VBOTraits<T>::element>& buffer,
	                                           const possumwood::ViewportState& vs) {
		             std::size_t index = 0;

		             for(auto& m : mesh) {
			             // vertex props, handled by polygon-triangle
			             if(m.vertexProperties().hasProperty(propertyName)) {
				             auto& vertProp = m.vertexProperties().property<T>(propertyName);

				             for(auto fit = m.polyhedron().facets_begin(); fit != m.polyhedron().facets_end(); ++fit) {
					             if(fit->facet_degree() > 2) {
						             auto it = fit->facet_begin();

						             const auto& val0 = extract(vertProp.get(it->vertex()->property_key()));
						             ++it;

						             for(unsigned ctr = 2; ctr < fit->facet_degree(); ++ctr) {
							             const auto& val1 = extract(vertProp.get(it->vertex()->property_key()));
							             ++it;
							             const auto& val2 = extract(vertProp.get(it->vertex()->property_key()));

							             buffer.element(index++) = val0;
							             buffer.element(index++) = val1;
							             buffer.element(index++) = val2;
						             }
					             }
				             }
			             }

			             // face props, constant for all triangles of a face
			             else if(m.faceProperties().hasProperty(propertyName)) {
				             auto& faceProp = m.faceProperties().property<T>(propertyName);

				             for(auto fit = m.polyhedron().facets_begin(); fit != m.polyhedron().facets_end(); ++fit) {
					             if(fit->facet_degree() > 2) {
						             auto n = extract(faceProp.get(fit->property_key()));

						             for(unsigned i = 2; i < fit->facet_degree(); ++i)
							             for(unsigned a = 0; a < 3; ++a)
								             buffer.element(index++) = n;
					             }
				             }
			             }

			             // halfedge props
			             else if(m.halfedgeProperties().hasProperty(propertyName)) {
				             auto& heProp = m.halfedgeProperties().property<T>(propertyName);

				             for(auto fit = m.polyhedron().facets_begin(); fit != m.polyhedron().facets_end(); ++fit) {
					             if(fit->facet_degree() > 2) {
						             auto it = fit->facet_begin();

						             const auto& val0 = extract(heProp.get(it->property_key()));
						             ++it;

						             for(unsigned ctr = 2; ctr < fit->facet_degree(); ++ctr) {
							             const auto& val1 = extract(heProp.get(it->property_key()));
							             ++it;
							             const auto& val2 = extract(heProp.get(it->property_key()));

							             buffer.element(index++) = val0;
							             buffer.element(index++) = val1;
							             buffer.element(index++) = val2;
						             }
					             }
				             }
			             }

			             else
				             throw std::runtime_error("Property " + propertyName +
				                                      " not found as halfedge, vertex or face property.");
		             }

		             assert(index == buffer.vertexCount());
	             });
}

struct VBOName {
	VBOName() : arraySize(0), recognized(false) {
	}

	std::string name, type;
	std::size_t arraySize;
	bool recognized;
};

VBOName CGALType(const std::string& t) {
	VBOName result;

	auto colonPos = t.find(":");
	if(colonPos != std::string::npos) {
		result.type = t.substr(0, colonPos);
		result.name = t.substr(colonPos + 1);
		result.arraySize = 1;
		result.recognized = true;

		assert(colonPos > 0);
		if(result.type.back() == ']') {
			auto bracketPos = t.find("[");
			assert(bracketPos != std::string::npos);

			result.arraySize = std::stoi(result.type.substr(bracketPos + 1, result.type.length() - bracketPos - 2));
			result.type = result.type.substr(0, bracketPos);
		}
	}

	return result;
}

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

dependency_graph::OutAttr<possumwood::VertexData> a_vd;
dependency_graph::InAttr<Meshes> a_mesh;
dependency_graph::InAttr<std::string> a_pointsName;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const Meshes mesh = data.get(a_mesh);

	// we're drawing triangles
	possumwood::VertexData vd(GL_TRIANGLES);

	// first, figure out how many triangles we have
	//   there has to be a better way to do this, come on
	std::size_t triangleCount = 0;
	for(auto& m : mesh)
		for(auto it = m.polyhedron().facets_begin(); it != m.polyhedron().facets_end(); ++it)
			if(it->facet_degree() > 2)
				triangleCount += (it->facet_degree() - 2);

	// and build the buffers
	if(triangleCount > 0) {
		// transfer the position data
		addVerticesVBO(vd, data.get(a_pointsName), mesh, triangleCount);

		// count the properties - only properties consistent between all meshes can be
		// transfered. We don't care about the type of the properties, though - face or
		// vertex are both fine
		std::map<std::string, std::size_t> propertyCounters;
		for(auto& m : mesh) {
			auto vertPropList = m.vertexProperties().properties();
			for(auto& vp : vertPropList)
				++propertyCounters[vp];

			auto facePropList = m.faceProperties().properties();
			for(auto& fp : facePropList)
				++propertyCounters[fp];

			auto halfedgePropList = m.halfedgeProperties().properties();
			for(auto& hp : halfedgePropList)
				++propertyCounters[hp];
		}

		// and transfer all properties that are common to all meshes
		std::set<std::string> ignoredProperties, inconsistentProperties;
		for(auto& pc : propertyCounters)
			if(pc.second == mesh.size()) {
				// analyse the type
				VBOName name = CGALType(pc.first);
				if(name.recognized && name.arraySize > 0) {
					// yep, the types are hardcoded for now, sorry :(
					if(name.type == "vec3" && name.arraySize == 1)
						addPerPointVBO<std::array<float, 3>>(vd, name.name, pc.first, triangleCount, mesh);

					else if(name.type == "vec2" && name.arraySize == 1)
						addPerPointVBO<std::array<float, 2>>(vd, name.name, pc.first, triangleCount, mesh);

					else if(name.type == "float" && name.arraySize == 1)
						addPerPointVBO<float>(vd, name.name, pc.first, triangleCount, mesh);

					else if(name.type == "int" && name.arraySize == 1)
						addPerPointVBO<int>(vd, name.name, pc.first, triangleCount, mesh);

					else if(name.type == "float")
						for(std::size_t i = 0; i < name.arraySize; ++i) {
							auto fn = [i](const std::vector<float>& v) -> float { return v[i]; };

							addPerPointVBO<std::vector<float>>(vd, name.name + "[" + std::to_string(i) + "]", pc.first,
							                                   triangleCount, mesh, fn);
						}

					else if(name.type == "int")
						for(std::size_t i = 0; i < name.arraySize; ++i) {
							auto fn = [i](const std::vector<int>& v) -> int { return v[i]; };

							addPerPointVBO<std::vector<int>>(vd, name.name + "[" + std::to_string(i) + "]", pc.first,
							                                 triangleCount, mesh, fn);
						}

					else
						ignoredProperties.insert(pc.first);
				}
				else
					ignoredProperties.insert(pc.first);
			}
			else
				inconsistentProperties.insert(pc.first + "(" + std::to_string(pc.second) + "/" +
				                              std::to_string(mesh.size()) + ")");

		if(!inconsistentProperties.empty())
			result.addWarning(
			    "The following properties were not common to all meshes, and had to be "
			    "ignored:\n" +
			    boost::algorithm::join(inconsistentProperties, ", "));

		if(!ignoredProperties.empty())
			result.addWarning("The following property types were not recognized:\n" +
			                  boost::algorithm::join(ignoredProperties, ", "));
	}

	data.set(a_vd, std::move(vd));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_mesh, "mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pointsName, "p_attr_name", std::string("P"));
	meta.addAttribute(a_vd, "vertex_data");

	meta.addInfluence(a_mesh, a_vd);
	meta.addInfluence(a_pointsName, a_vd);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/vertex_data/cgal", init);
}  // namespace
