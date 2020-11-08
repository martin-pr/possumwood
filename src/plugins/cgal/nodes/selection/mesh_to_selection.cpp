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

dependency_graph::InAttr<Meshes> a_mesh;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<FaceSelection> a_selection;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	FaceSelection selection;

	std::size_t i = 0;
	for(auto mesh : data.get(a_mesh)) {
		FaceSelection::Item item(mesh);

		if(data.get(a_mode).intValue() == 0 /* all faces */) {
			for(auto h = mesh.polyhedron().facets_begin(); h != mesh.polyhedron().facets_end(); ++h)
				item.push_back(h);
		}

		selection.push_back(item);

		++i;
	}

	data.set(a_selection, selection);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_mesh, "mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"Select all faces", "Select nothing"}));
	meta.addAttribute(a_selection, "selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_mesh, a_selection);
	meta.addInfluence(a_mode, a_selection);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/selection/mesh_to_selection", init);
}  // namespace
