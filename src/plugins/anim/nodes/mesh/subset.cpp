#include <possumwood_sdk/node_implementation.h>

#include "datatypes/skinned_mesh.h"
#include "datatypes/subset_selection.h"

namespace {

dependency_graph::InAttr<anim::SubsetSelection> a_subset;
dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_inMeshes;
dependency_graph::OutAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_outMeshes;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::shared_ptr<const std::vector<anim::SkinnedMesh>> inMeshes = data.get(a_inMeshes);
	if(!inMeshes) {
		data.set(a_outMeshes, std::shared_ptr<const std::vector<anim::SkinnedMesh>>());
		data.set(a_subset, anim::SubsetSelection());
	}
	else {
		{
			// build the options from the current input
			anim::SubsetSelection::Options options;
			for(auto& m : *inMeshes)
				options.add(m.name());

			// update a_editorData, if needed
			if(data.get(a_subset).options() != options) {
				anim::SubsetSelection subsetData = data.get(a_subset);
				subsetData.options() = options;
				data.set(a_subset, subsetData);
			}
		}

		// get the selection
		const anim::SubsetSelection& selection = data.get(a_subset);
		std::shared_ptr<const std::vector<anim::SkinnedMesh>> inMeshes = data.get(a_inMeshes);

		// construct the outmeshes set
		std::unique_ptr<std::vector<anim::SkinnedMesh>> outMeshes(new std::vector<anim::SkinnedMesh>());
		for(auto& m : *inMeshes) {
			auto it = selection.find(m.name());
			assert(it != selection.end());
			if(it->second)
				outMeshes->push_back(m);
		}

		// and stick it to the output
		data.set(a_outMeshes, std::shared_ptr<const std::vector<anim::SkinnedMesh>>(outMeshes.release()));
	}

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_subset, "subset");
	meta.addAttribute(a_inMeshes, "in_meshes", std::shared_ptr<const std::vector<anim::SkinnedMesh>>(),
	                  possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outMeshes, "out_meshes", std::shared_ptr<const std::vector<anim::SkinnedMesh>>(),
	                  possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_subset, a_outMeshes);
	meta.addInfluence(a_inMeshes, a_outMeshes);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/mesh/subset", init);

}  // namespace
