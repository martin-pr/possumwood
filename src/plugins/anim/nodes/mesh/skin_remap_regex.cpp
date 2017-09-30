#include <regex>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/skinned_mesh.h"
#include "datatypes/skeleton.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_skeleton;
dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_inMeshes;
dependency_graph::InAttr<std::string> a_regex, a_replacement;
dependency_graph::OutAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_outMeshes;

dependency_graph::State compute(dependency_graph::Values& data) {
	const anim::Skeleton& skeleton = data.get(a_skeleton);

	const std::regex regex(data.get(a_regex));
	const std::string& format = data.get(a_replacement);

	dependency_graph::State state;

	// assemble the bone map
	std::vector<unsigned> mapping(skeleton.size());
	unsigned mappedCount = 0;
	for(std::size_t i=0;i<skeleton.size();++i) {
		// try to match the regex, and perform replacement
		const std::string result = std::regex_replace(skeleton[i].name(), regex, format);

		// by default, just use identity
		mapping[i] = i;

		// if a replacement was done
		if(result != skeleton[i].name()) {
			// try to find a match for the replacement result
			int targetIndex = -1;
			for(std::size_t j=0;j<skeleton.size();++j)
				if(skeleton[j].name() == result) {
					if(targetIndex == -1)
						targetIndex = j;
					else {
						std::stringstream err;
						err << "Bone " << skeleton[i].name() << " matched simultaneously with "
							<< skeleton[targetIndex].name() << " and " << skeleton[j].name() << std::endl;

						state.addWarning(err.str());
					}
				}

			// if a match is found, record it in the bone map
			if(targetIndex < 0) {
				std::stringstream err;
				err << "Bone " << skeleton[i].name() << " matched the search criteria, but its replacement result "
					<< result << " was not found in the skeleton hierarchy";

				state.addWarning(err.str());
			}
			else {
				mapping[i] = targetIndex;
				++mappedCount;
			}
		}
	}

	{
		std::stringstream info;
		info << "Mapped " << mappedCount << " bones.";
		state.addInfo(info.str());
	}

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

	return state;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_skeleton, "skeleton");
	meta.addAttribute(a_inMeshes, "in_meshes");
	meta.addAttribute(a_regex, "regex", std::string("(.*)"));
	meta.addAttribute(a_replacement, "replacement", std::string("$1"));
	meta.addAttribute(a_outMeshes, "out_meshes");

	meta.addInfluence(a_skeleton, a_outMeshes);
	meta.addInfluence(a_inMeshes, a_outMeshes);
	meta.addInfluence(a_regex, a_outMeshes);
	meta.addInfluence(a_replacement, a_outMeshes);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/mesh/skin_remap_regex", init);

}
