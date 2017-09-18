#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include "datatypes/skinned_mesh.h"
#include "datatypes/joint_mapping_editor_data.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_skeleton;
dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_inMeshes;
dependency_graph::InAttr<anim::JointMappingEditorData> a_editorData;
dependency_graph::OutAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_outMeshes;

dependency_graph::State compute(dependency_graph::Values& data) {
	const anim::Skeleton& skeleton = data.get(a_skeleton);

	// update a_editorData, if needed
	if(data.get(a_editorData).sourceSkeleton() != skeleton) {
		anim::JointMappingEditorData editorData = data.get(a_editorData);
		editorData.setSourceSkeleton(skeleton);
		editorData.setTargetSkeleton(skeleton);
		data.set(a_editorData, editorData);
	}

	// make the remapping vector
	std::vector<int> mapping(skeleton.size(), -1);
	for(auto& m : data.get(a_editorData))
		if(m.first >= 0 && m.first < (int)mapping.size() && m.second >= 0 && m.second < (int)mapping.size())
			mapping[m.first] = m.second;

	// do the remapping
	std::unique_ptr<std::vector<anim::SkinnedMesh>> result(new std::vector<anim::SkinnedMesh>());

	std::shared_ptr<const std::vector<anim::SkinnedMesh>> inMeshes = data.get(a_inMeshes);
	if(inMeshes != nullptr)
		for(auto& m : *inMeshes) {
			result->push_back(m);

			auto& mesh = result->back();

			for(auto& v : mesh.vertices())
				for(auto& w : v)
					if(w.first < mapping.size() && mapping[w.first] >= 0)
						w.first = mapping[w.first];
		}

	data.set(a_outMeshes, std::shared_ptr<const std::vector<anim::SkinnedMesh>>(result.release()));

	return dependency_graph::State();

}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_skeleton, "skeleton");
	meta.addAttribute(a_inMeshes, "in_meshes");
	meta.addAttribute(a_editorData, "mapping");
	meta.addAttribute(a_outMeshes, "out_meshes");

	meta.addInfluence(a_skeleton, a_outMeshes);
	meta.addInfluence(a_inMeshes, a_outMeshes);
	meta.addInfluence(a_editorData, a_outMeshes);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/skin_remapping", init);

}
