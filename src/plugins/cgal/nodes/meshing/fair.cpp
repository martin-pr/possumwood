#include <CGAL/Polygon_mesh_processing/fair.h>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/meshes.h"
#include "datatypes/selection.h"
#include "errors.h"

namespace {

using possumwood::FaceSelection;

dependency_graph::InAttr<FaceSelection> a_inSelection;
dependency_graph::InAttr<unsigned> a_continuity;
dependency_graph::OutAttr<FaceSelection> a_outSelection;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	FaceSelection result;
	for(const auto& current : data.get(a_inSelection)) {
		auto mesh = current.mesh();
		{
			auto& editableMesh = mesh.edit();

			std::set<possumwood::CGALPolyhedron::Vertex_handle> handles;
			for(auto it = editableMesh.polyhedron().facets_begin(); it != editableMesh.polyhedron().facets_end(); ++it)
				if(current.contains(it)) {
					auto vit = it->facet_begin();
					for(std::size_t a=0; a<it->facet_degree(); ++a, ++vit)
						handles.insert(vit->vertex());
				}

			CGAL::Polygon_mesh_processing::fair(
				editableMesh.polyhedron(), handles,
				CGAL::Polygon_mesh_processing::parameters::fairing_continuity(data.get(a_continuity)));
		}

		FaceSelection::Item item = current;
		item.setMesh(mesh);

		result.push_back(item);
	}

	data.set(a_outSelection, result);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSelection, "in_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_continuity, "continuity", 2u);
	meta.addAttribute(a_outSelection, "out_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inSelection, a_outSelection);
	meta.addInfluence(a_continuity, a_outSelection);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/meshing/fair", init);
}  // namespace
