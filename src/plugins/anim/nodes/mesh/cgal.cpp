#include "cgal.h"

#include <possumwood_sdk/node_implementation.h>

#include <regex>

#include "builder.h"
#include "datatypes/meshes.h"
#include "datatypes/skeleton.h"
#include "datatypes/skinned_mesh.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_inMeshes;

dependency_graph::InAttr<unsigned> a_skinningCount;
dependency_graph::OutAttr<Meshes> a_outMeshes;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::shared_ptr<const std::vector<anim::SkinnedMesh>> meshes = data.get(a_inMeshes);

	Meshes result;

	if(meshes != nullptr) {
		for(auto& m : *meshes) {
			std::vector<possumwood::CGALKernel::Point_3> vertices;
			for(auto& v : m.vertices())
				vertices.push_back(possumwood::CGALKernel::Point_3(v.pos().x, v.pos().y, v.pos().z));

			possumwood::Mesh& out = result.addMesh(m.name());

			{
				possumwood::CGALBuilder<possumwood::CGALPolyhedron::HalfedgeDS, typeof(vertices), typeof(m.polygons())>
				    builder(vertices, m.polygons());
				out.polyhedron().delegate(builder);
			}

			// make a vector property for skinning
			const std::string skinWeightsPropName =
			    "float[" + std::to_string(data.get(a_skinningCount)) + "]:skinningWeights";
			const std::string skinIndicesPropName =
			    "int[" + std::to_string(data.get(a_skinningCount)) + "]:skinningIndices";

			auto& skinWeightsProp = out.vertexProperties().addProperty(skinWeightsPropName, std::vector<float>());
			auto& skinIndicesProp = out.vertexProperties().addProperty(skinIndicesPropName, std::vector<int>());

			std::size_t vertexIndex = 0;
			for(auto vi = out.polyhedron().vertices_begin(); vi != out.polyhedron().vertices_end(); ++vi) {
				std::vector<float> weights(data.get(a_skinningCount), 0.0f);
				std::vector<int> indices(data.get(a_skinningCount), 0);

				anim::Skinning skin = m.vertices()[vertexIndex].skinning();
				skin.limitInfluenceCount(data.get(a_skinningCount));
				assert(skin.size() <= data.get(a_skinningCount));

				std::size_t ctr = 0;
				for(auto& w : skin) {
					indices[ctr] = w.bone;
					weights[ctr] = w.weight;

					++ctr;
				}

				skinWeightsProp.set(vi->property_key(), weights);
				skinIndicesProp.set(vi->property_key(), indices);

				++vertexIndex;
			}
		}
	}

	data.set(a_outMeshes, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMeshes, "in_meshes");
	meta.addAttribute(a_skinningCount, "skinning_count", 4u);
	meta.addAttribute(a_outMeshes, "out_cgal");

	meta.addInfluence(a_inMeshes, a_outMeshes);
	meta.addInfluence(a_skinningCount, a_outMeshes);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/mesh/to_cgal", init);
}  // namespace
