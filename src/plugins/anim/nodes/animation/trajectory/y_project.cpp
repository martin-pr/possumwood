#include <possumwood_sdk/node_implementation.h>

#include <utility>
#include <random>

#include <possumwood_sdk/app.h>

#include "datatypes/skeleton.h"
#include "datatypes/animation.h"
#include "datatypes/motion_graph.h"
#include "ui/motion_map.h"

namespace {

dependency_graph::InAttr<anim::Animation> a_inAnim;
dependency_graph::OutAttr<anim::Animation> a_outAnim;
dependency_graph::InAttr<float> a_groundLevel;

dependency_graph::State compute(dependency_graph::Values& values) {
	const anim::Animation& inAnim = values.get(a_inAnim);

	anim::Animation result(inAnim.fps());

	if(!inAnim.empty()) {
		// create the "base" frame - either a copy of the first frame, or a frame with "__trajectory__" as root
		anim::Skeleton frame(inAnim.front());
		bool createdNewTrajectoryBone = false;

		if(!frame.empty()) {
			if(frame[0].name() != "__trajectory__") {
				frame.addRoot("__trajectory__", anim::Transform());
				createdNewTrajectoryBone = true;
			}

			// copy the animation
			for(auto& f : inAnim) {
				for(std::size_t bi=0; bi<f.size(); ++bi)
					frame[bi + createdNewTrajectoryBone].tr() = f[bi].tr();

				// and set the trajectory bone to be the original trajectory
				anim::Transform root = frame[1].tr();
				// with Y = 0
				root.translation.y = 0.0f;
				// and with only rotation around Y axis
				{
					Imath::V3f rotated = Imath::V3f(1,0,0) * root.rotation;
					rotated.y = 0;
					rotated.normalize();

					root.rotation.setRotation(Imath::V3f(1,0,0), rotated);
				}

				frame[0].tr() = root;
				frame[1].tr() = root.inverse() * frame[1].tr();

				result.addFrame(frame);
			}
		}
	}

	values.set(a_outAnim, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	std::random_device rd;

	meta.addAttribute(a_inAnim, "in_anim", anim::Animation(24.0f), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outAnim, "out_anim", anim::Animation(24.0f), possumwood::AttrFlags::kVertical);

	meta.addAttribute(a_groundLevel, "ground_level", 0.0f);

	meta.addInfluence(a_inAnim, a_outAnim);
	meta.addInfluence(a_groundLevel, a_outAnim);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/animation/trajectory/y_project", init);

}
