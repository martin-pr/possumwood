#include <possumwood_sdk/node_implementation.h>

#include <possumwood_sdk/app.h>

#include "datatypes/constraints.h"
#include "datatypes/animation.h"

namespace {

dependency_graph::InAttr<anim::Constraints> a_inConstraints;
dependency_graph::InAttr<unsigned> a_width;
dependency_graph::OutAttr<anim::Constraints> a_outConstraints;

dependency_graph::State compute(dependency_graph::Values& data) {
	anim::Constraints constraints = data.get(a_inConstraints);
	const int width = data.get(a_width);

	std::vector<float> result;
	std::vector<float> tmp;

	for(auto& j : constraints) {
		constraints.process(j.first, [&](anim::constraints::Frames& frames) {
			result.clear();

			for(int frameIndex=0; frameIndex < (int)frames.size(); ++frameIndex) {
				const int start = std::max(frameIndex - width/2, 0);
				const int end = std::min(frameIndex + width/2, (int)frames.size() - 1);

				tmp.resize(end-start+1);
				for(int a=start;a<=end;++a)
					tmp[a-start] = frames[a].value();
				std::sort(tmp.begin(), tmp.end());

				result.push_back(tmp[tmp.size()/2]);
			}

			assert(result.size() == frames.size());

			auto it = result.begin();
			for(auto& frame : frames)
				frame.setValue(*it++);
		});
	}

	std::cout << "Median:" << std::endl;
	std::cout << constraints << std::endl;
	std::cout << std::endl;

	data.set(a_outConstraints, constraints);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inConstraints, "in_constraints", anim::Constraints(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_width, "median_width", 7u);
	meta.addAttribute(a_outConstraints, "out_constraints", anim::Constraints(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inConstraints, a_outConstraints);
	meta.addInfluence(a_width, a_outConstraints);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/constraints/filter_median", init);

}
