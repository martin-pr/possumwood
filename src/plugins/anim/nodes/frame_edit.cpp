#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathEuler.h>

#include "datatypes/animation.h"
#include "datatypes/frame_editor_data.h"
#include "3d/io/vec3.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_inFrame;
dependency_graph::InAttr<Imath::Vec3<float>> a_rootTranslation;
dependency_graph::InAttr<anim::FrameEditorData> a_editorData;
dependency_graph::OutAttr<anim::Skeleton> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	// update a_editorData, if needed
	if(data.get(a_editorData).skeleton() != data.get(a_inFrame)) {
		anim::FrameEditorData editorData = data.get(a_editorData);
		editorData.setSkeleton(data.get(a_inFrame));
		data.set(a_editorData, editorData);
	}

	// do the computation itself
	anim::Skeleton frame = data.get(a_inFrame);
	const Imath::Vec3<float>& rootTr = data.get(a_rootTranslation);

	if(frame.size() > 0) {
		frame[0].tr().translation += rootTr;

		for(auto& i : data.get(a_editorData))
			frame[i.first].tr() = i.second * frame[i.first].tr();
	}

	data.set(a_outFrame, frame);

	return dependency_graph::State();

}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame");
	meta.addAttribute(a_rootTranslation, "root_translate", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_editorData, "rotations");
	meta.addAttribute(a_outFrame, "out_frame");

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_editorData, a_outFrame);
	meta.addInfluence(a_rootTranslation, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/frame_edit", init);

}
