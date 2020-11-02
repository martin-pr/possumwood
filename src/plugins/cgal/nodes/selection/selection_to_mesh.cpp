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

dependency_graph::InAttr<FaceSelection> a_selection;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<Meshes> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	Meshes meshes;

	for(auto& item : data.get(a_selection)) {
		possumwood::Mesh mesh = item.mesh();

		{
			auto& edit = mesh.edit();

			std::vector<possumwood::CGALPolyhedron::Facet_handle> handles;

			auto it = edit.polyhedron().facets_begin();
			if(data.get(a_mode).value() == "Keep selected")
				for(auto f = edit.polyhedron().facets_begin(); f != edit.polyhedron().facets_end(); ++f, ++it) {
					if(!item.contains(f))
						handles.push_back(it);
				}
			else
				for(auto f = edit.polyhedron().facets_begin(); f != edit.polyhedron().facets_end(); ++f, ++it) {
					if(item.contains(f))
						handles.push_back(it);
				}

			for(auto& h : handles)
				edit.polyhedron().erase_facet(h->halfedge());
		}

		meshes.addMesh(mesh);
	}

	data.set(a_mesh, meshes);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_selection, "selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"Keep selected", "Remove selected"}));
	meta.addAttribute(a_mesh, "mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_selection, a_mesh);
	meta.addInfluence(a_mode, a_mesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/selection/selection_to_mesh", init);
}  // namespace
