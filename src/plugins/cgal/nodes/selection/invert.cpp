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

dependency_graph::InAttr<FaceSelection> a_inSelection;
dependency_graph::OutAttr<FaceSelection> a_outSelection;

dependency_graph::State compute(dependency_graph::Values& data) {
	FaceSelection result;

	for(auto& in : data.get(a_inSelection)) {
		FaceSelection::Item out = in;
		out.clear();

		for(auto fit = in.mesh().polyhedron().facets_begin(); fit != in.mesh().polyhedron().facets_end(); ++fit)
			if(!in.contains(fit))
				out.push_back(fit);

		result.push_back(out);
	}

	data.set(a_outSelection, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSelection, "in_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outSelection, "out_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inSelection, a_outSelection);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/selection/invert", init);

}  // namespace
