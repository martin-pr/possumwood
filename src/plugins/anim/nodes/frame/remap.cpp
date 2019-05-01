#include <possumwood_sdk/node_implementation.h>

#include "datatypes/skeleton.h"
#include "datatypes/joint_mapping_editor_data.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_inFrame, a_sourceSkeleton, a_targetSkeleton;
dependency_graph::InAttr<anim::JointMappingEditorData> a_mapping;
dependency_graph::OutAttr<anim::Skeleton> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	// update a_mapping, if needed
	if(data.get(a_mapping).sourceSkeleton() != data.get(a_sourceSkeleton) ||
	   data.get(a_mapping).targetSkeleton() != data.get(a_targetSkeleton)) {

		anim::JointMappingEditorData editorData = data.get(a_mapping);
		editorData.setSourceSkeleton(data.get(a_sourceSkeleton));
		editorData.setTargetSkeleton(data.get(a_targetSkeleton));
		data.set(a_mapping, editorData);
	}

	// get the two skeletons
	anim::Skeleton source = data.get(a_sourceSkeleton);
	anim::Skeleton target = data.get(a_targetSkeleton);

	anim::Skeleton inFrame = data.get(a_inFrame);

	if(!inFrame.empty() && !target.empty() && !source.empty()) {
		if(!inFrame.isCompatibleWith(source))
			throw std::runtime_error(
					  "Source base skeleton and Frame have to be compatible for mapping to work.");

		anim::Skeleton output = target;
		const anim::JointMappingEditorData& mapping = data.get(a_mapping);

		// convert source and target to world space (A matrices)
		for(auto& j : source)
			if(j.hasParent())
				j.tr() = j.parent().tr() * j.tr();
		for(auto& j : target)
			if(j.hasParent())
				j.tr() = j.parent().tr() * j.tr();

		// convert the in frame to world space (F matrices)
		for(auto& j : inFrame)
			if(j.hasParent())
				j.tr() = j.parent().tr() * j.tr();

		// build the world space output by converting the frame into world space
		for(auto& j : output) {
			// world space propagation, based on the animated frame's data
			if(j.hasParent())
				j.tr() = j.parent().tr() * j.tr();

			auto mapIt = mapping.findTarget(j.index());
			if(mapIt != mapping.end() && mapIt->first >= 0 && mapIt->first < (int)source.size() &&
			   mapIt->second >= 0 && mapIt->second < (int)target.size()) {

				assert((int)j.index() == mapIt->second);

				auto sourceJoint = source[mapIt->first];
				auto animJoint = inFrame[mapIt->first];
				auto targetJoint = target[mapIt->second];

				// map using the skinning matrix
				j.tr() = animJoint.tr() * sourceJoint.tr().inverse() * targetJoint.tr();
			}
		}

		// back to local space
		for(unsigned bi = output.size()-1; bi > 0; --bi) {
			auto& jnt = output[bi];
			assert(jnt.hasParent());

			jnt.tr() = jnt.parent().tr().inverse() * jnt.tr();
		}

		// and put the output to the output
		data.set(a_outFrame, output);
	}

	else
		data.set(a_outFrame, anim::Skeleton());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", anim::Skeleton(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_sourceSkeleton, "source_skeleton");
	meta.addAttribute(a_targetSkeleton, "target_skeleton");
	meta.addAttribute(a_mapping, "mapping");
	meta.addAttribute(a_outFrame, "out_frame", anim::Skeleton(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_sourceSkeleton, a_outFrame);
	meta.addInfluence(a_targetSkeleton, a_outFrame);
	meta.addInfluence(a_mapping, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/frame/remap", init);

}
