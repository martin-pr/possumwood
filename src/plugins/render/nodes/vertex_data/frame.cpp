#include <GL/glew.h>
#include <GL/glu.h>
#include <OpenEXR/ImathVec.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/node_implementation.h>

#include "anim/datatypes/skeleton.h"
#include "datatypes/vertex_data.inl"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_skeleton;
dependency_graph::OutAttr<possumwood::VertexData> a_vd;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	// we're drawing lines this time
	possumwood::VertexData vd(GL_LINES);
	anim::Skeleton skeleton = data.get(a_skeleton);

	if(!skeleton.empty()) {
		// convert the skeleton to global
		for(auto& b : skeleton)
			if(b.hasParent())
				b.tr() = b.parent().tr() * b.tr();

		// there will be one position vector per bone. Root will simply be zero-length
		vd.addVBO<Imath::V3f>("P", skeleton.size() * 2, possumwood::VertexData::kStatic,
		                      [skeleton](possumwood::Buffer<float>& buffer, const possumwood::ViewportState& vs) {
			                      std::size_t ctr = 0;
			                      for(auto& b : skeleton) {
				                      buffer.element(ctr++) = b.tr().translation;
				                      if(b.hasParent())
					                      buffer.element(ctr++) = b.parent().tr().translation;
				                      else
					                      buffer.element(ctr++) = b.tr().translation;
			                      }
			                      assert(ctr == skeleton.size() * 2);
		                      });

		// there will be one position vector per bone. Root will simply be zero-length
		vd.addVBO<int>("bone_id", skeleton.size() * 2, possumwood::VertexData::kStatic,
		               [skeleton](possumwood::Buffer<int>& buffer, const possumwood::ViewportState& vs) {
			               std::size_t ctr = 0;
			               for(auto& b : skeleton) {
				               buffer.element(ctr++) = (int)b.index();
				               if(b.hasParent())
					               buffer.element(ctr++) = (int)b.parent().index();
				               else
					               buffer.element(ctr++) = (int)b.index();
			               }
			               assert(ctr == skeleton.size() * 2);
		               });
	}

	data.set(a_vd, std::move(vd));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_skeleton, "frame");
	meta.addAttribute(a_vd, "vertex_data");

	meta.addInfluence(a_skeleton, a_vd);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/vertex_data/anim_frame", init);

}  // namespace
