// #include <regex>

// #include <possumwood_sdk/node_implementation.h>

// #include "datatypes/skeleton.h"
// #include "datatypes/skinned_mesh.h"
// #include "datatypes/meshes.h"
// #include "cgal.h"

// namespace {

// using possumwood::Meshes;
// using possumwood::CGALPolyhedron;

// dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>>
//     a_inMeshes;

// dependency_graph::InAttr<unsigned> a_skinningCount;
// dependency_graph::OutAttr<Meshes> a_outMeshes;

// dependency_graph::State compute(dependency_graph::Values& data) {
// 	std::shared_ptr<const std::vector<anim::SkinnedMesh>> meshes =
// 	    data.get(a_inMeshes);

// 	Meshes result;

// 	if(meshes != nullptr) {
// 		for(auto& m : *meshes) {
// 			std::unique_ptr<possumwood::CGALPolyhedron> out(
// 			    new possumwood::CGALPolyhedron());

// 			for(auto& v : m.vertices()) {
// 				const possumwood::CGALKernel::Point_3 pt(v.pos().x, v.pos().y,
// 				                                         v.pos().z);
// 				out->add_vertex(pt);
// 			}

// 			for(auto& p : m.polygons())
// 				out->add_face(p);

// 			// make a vector property for skinning
// 			const std::string skinWeightsPropName = "float[" + std::to_string(data.get(a_skinningCount)) + "]:skinningWeights";
// 			const std::string skinIndicesPropName = "int[" + std::to_string(data.get(a_skinningCount)) + "]:skinningIndices";

// 			auto skinWeightsProp = out->add_property_map<CGALPolyhedron::Vertex_index, std::vector<float>>(skinWeightsPropName);
// 			auto skinIndicesProp = out->add_property_map<CGALPolyhedron::Vertex_index, std::vector<int>>(skinIndicesPropName);

// 			std::size_t vertexIndex = 0;
// 			for(auto& v : out->vertices()) {
// 				std::vector<float> weights(data.get(a_skinningCount), 0.0f);
// 				std::vector<int> indices(data.get(a_skinningCount), 0);

// 				anim::Skinning skin = m.vertices()[vertexIndex].skinning();
// 				skin.limitInfluenceCount(data.get(a_skinningCount));
// 				assert(skin.size() <= data.get(a_skinningCount));

// 				std::size_t ctr = 0;
// 				for(auto& w : skin) {
// 					indices[ctr] = w.bone;
// 					weights[ctr] = w.weight;

// 					++ctr;
// 				}

// 				skinWeightsProp.first[v] = weights;
// 				skinIndicesProp.first[v] = indices;

// 				++vertexIndex;
// 			}


// 			result.addMesh(m.name(), std::move(out));
// 		}
// 	}

// 	data.set(a_outMeshes, result);

// 	return dependency_graph::State();
// }

// void init(possumwood::Metadata& meta) {
// 	meta.addAttribute(a_inMeshes, "in_meshes");
// 	meta.addAttribute(a_skinningCount, "skinning_count", 4u);
// 	meta.addAttribute(a_outMeshes, "out_cgal");

// 	meta.addInfluence(a_inMeshes, a_outMeshes);
// 	meta.addInfluence(a_skinningCount, a_outMeshes);

// 	meta.setCompute(compute);
// }

// possumwood::NodeImplementation s_impl("anim/mesh/to_cgal", init);
// }
