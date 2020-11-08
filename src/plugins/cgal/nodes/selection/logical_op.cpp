#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include "datatypes/meshes.h"
#include "datatypes/selection.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::FaceSelection;
using possumwood::Meshes;

typedef possumwood::CGALPolyhedron Mesh;

dependency_graph::InAttr<FaceSelection> a_inSelection1;
dependency_graph::InAttr<FaceSelection> a_inSelection2;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<FaceSelection> a_outSelection;

enum Mode { kAnd, kOr, kXor };

std::vector<std::pair<std::string, Mode>> s_modes{{"And", kAnd}, {"Or", kOr}, {"Xor", kXor}};

dependency_graph::State compute(dependency_graph::Values& data) {
	FaceSelection result;

	const auto& in1 = data.get(a_inSelection1);
	const auto& in2 = data.get(a_inSelection2);

	if(in1.size() != in2.size())
		throw std::runtime_error("Input meshes must be the same.");

	for(std::size_t i = 0; i < in1.size(); ++i) {
		const auto& inSel1 = in1[i];
		const auto& inSel2 = in2[i];

		if(inSel1.mesh() != inSel2.mesh())
			throw std::runtime_error("Input meshes must be the same.");

		FaceSelection::Item out = inSel1;
		out.clear();

		switch(data.get(a_mode).intValue()) {
			case kAnd:
				for(auto fit = out.mesh().polyhedron().facets_begin(); fit != out.mesh().polyhedron().facets_end();
				    ++fit)
					if(inSel1.contains(fit) && inSel2.contains(fit))
						out.push_back(fit);
				break;
			case kOr:
				for(auto fit = out.mesh().polyhedron().facets_begin(); fit != out.mesh().polyhedron().facets_end();
				    ++fit)
					if(inSel1.contains(fit) || inSel2.contains(fit))
						out.push_back(fit);
				break;
			case kXor:
				for(auto fit = out.mesh().polyhedron().facets_begin(); fit != out.mesh().polyhedron().facets_end();
				    ++fit)
					if(inSel1.contains(fit) != inSel2.contains(fit))
						out.push_back(fit);
				break;
		}

		result.push_back(out);
	}

	data.set(a_outSelection, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSelection1, "in_selection_1", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inSelection2, "in_selection_2", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_modes.begin(), s_modes.end()));
	meta.addAttribute(a_outSelection, "out_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inSelection1, a_outSelection);
	meta.addInfluence(a_inSelection2, a_outSelection);
	meta.addInfluence(a_mode, a_outSelection);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/selection/logical_op", init);

}  // namespace
