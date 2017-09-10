#include <regex>

#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include "datatypes/skeleton.h"
#include "datatypes/skinned_mesh.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_baseSkeleton;
dependency_graph::InAttr<anim::Skeleton> a_posedSkeleton;
dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_inMeshes;
dependency_graph::OutAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_posedMeshes;

dependency_graph::State compute(dependency_graph::Values& data) {
	anim::Skeleton baseSkeleton = data.get(a_baseSkeleton);
	anim::Skeleton posedSkeleton = data.get(a_posedSkeleton);
	std::shared_ptr<const std::vector<anim::SkinnedMesh>> meshes = data.get(a_inMeshes);

	if(!baseSkeleton.empty() && meshes != nullptr) {
		if(!baseSkeleton.isCompatibleWith(posedSkeleton))
			throw std::runtime_error("Base and posed skeletons don't seem to be compatible.");

		// assemble the skinning matrices

		// first, convert both skeletons to world space
		for(auto& b : baseSkeleton)
			if(b.hasParent())
				b.tr() = b.parent().tr() * b.tr();

		for(auto& b : posedSkeleton)
			if(b.hasParent())
				b.tr() = b.parent().tr() * b.tr();

		// and compute transformations for the skinning itself
		std::vector<Imath::M44f> transforms(baseSkeleton.size());
		for(unsigned bi=0;bi<transforms.size();++bi)
			transforms[bi] = (posedSkeleton[bi].tr().toMatrix44() * baseSkeleton[bi].tr().toMatrix44().inverse());

		// and do the skinning
		std::set<unsigned> missingBones;

		std::unique_ptr<std::vector<anim::SkinnedMesh>> posedMeshes(new std::vector<anim::SkinnedMesh>());
		for(auto& m : *meshes) {
			posedMeshes->push_back(m);
			anim::SkinnedMesh& mesh = posedMeshes->back();

			for(auto& v : mesh.vertices()) {
				Imath::V3f pos(0,0,0);
				float sum = 0.0f;

				for(auto& w : v) {
					if(w.first >= transforms.size())
						missingBones.insert(w.first);
					else {
						pos += (v.pos() * transforms[w.first].transposed()) * w.second;
						sum += w.second;
					}
				}

				if(sum > 0.0f)
					v.setPos(pos / sum);
			}
		}

		if(!missingBones.empty()) {
			std::stringstream err;
			err << "Skinning uses a bone that is not included in the skeleton! Missing bone IDs:";

			for(auto& b : missingBones)
				err << b << " ";

			throw std::runtime_error(err.str());
		}

		// move the output
		data.set(a_posedMeshes, std::shared_ptr<const std::vector<anim::SkinnedMesh>>(
			posedMeshes.release()));
	}
	else
		data.set(a_posedMeshes, std::shared_ptr<const std::vector<anim::SkinnedMesh>>());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_baseSkeleton, "base_skeleton");
	meta.addAttribute(a_posedSkeleton, "posed_skeleton");
	meta.addAttribute(a_inMeshes, "meshes");
	meta.addAttribute(a_posedMeshes, "posed_meshes");

	meta.addInfluence(a_baseSkeleton, a_posedMeshes);
	meta.addInfluence(a_posedSkeleton, a_posedMeshes);
	meta.addInfluence(a_inMeshes, a_posedMeshes);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/skin", init);

}
