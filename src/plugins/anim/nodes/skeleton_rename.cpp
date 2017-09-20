#include <regex>

#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include "datatypes/skeleton.h"

namespace {

dependency_graph::InAttr<anim::Skeleton> a_inSkeleton;
dependency_graph::InAttr<std::string> a_regex;
dependency_graph::OutAttr<anim::Skeleton> a_outSkeleton;

std::string processName(const std::string& name, const std::regex& regex) {
	std::smatch match;
	if(std::regex_search(name, match, regex))
		return match[0];
	else
		return name;
};

dependency_graph::State compute(dependency_graph::Values& data) {
	const std::regex regex(data.get(a_regex));
	const anim::Skeleton inSkeleton = data.get(a_inSkeleton);

	anim::Skeleton outSkeleton;

	if(!inSkeleton.empty()) {
		outSkeleton.addRoot(processName(inSkeleton[0].name(), regex), inSkeleton[0].tr());

		for(unsigned a=1;a<inSkeleton.size();++a)
			outSkeleton.addChild(outSkeleton[inSkeleton[a].parent().index()],
				inSkeleton[a].tr(), processName(inSkeleton[a].name(), regex));
	}

	data.set(a_outSkeleton, outSkeleton);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSkeleton, "in_skeleton");
	meta.addAttribute(a_regex, "regex", std::string(".*"));
	meta.addAttribute(a_outSkeleton, "out_skeleton");

	meta.addInfluence(a_inSkeleton, a_outSkeleton);
	meta.addInfluence(a_regex, a_outSkeleton);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/skeleton_rename", init);

}
