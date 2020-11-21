#include <possumwood_sdk/node_implementation.h>

#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>

#include "datatypes/selection.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::FaceSelection;

dependency_graph::InAttr<FaceSelection> a_in;
dependency_graph::InAttr<float> a_targetEdgeLength;
dependency_graph::InAttr<unsigned> a_iterations, a_relaxationSteps;
dependency_graph::OutAttr<FaceSelection> a_out;

// a workaround for namespace changes in CGAL - parameters namespace for polygon mesh processing has changed
// from Ubuntu 18.04 to 20.10
#if CGAL_VERSION_MAJOR >= 5
using namespace CGAL::parameters;
#else
using namespace CGAL::Polygon_mesh_processing::parameters;
#endif

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	FaceSelection result;

	for(auto& item : data.get(a_in)) {
		auto mesh = item.mesh();
		auto& editable = mesh.edit();
		auto& poly = editable.polyhedron();

		std::vector<possumwood::CGALPolyhedron::Facet_handle> input;
		for(auto h = poly.facets_begin(); h != poly.facets_end(); ++h)
			if(item.contains(h))
				input.push_back(h);

		std::vector<possumwood::CGALPolyhedron::Facet_const_handle> output;

		auto params = number_of_iterations(data.get(a_iterations))
		                  .number_of_relaxation_steps(data.get(a_relaxationSteps))
		                  .protect_constraints(false)
		                  .vertex_index_map(CGAL::get(CGAL::vertex_external_index, poly))
		                  .halfedge_index_map(CGAL::get(CGAL::halfedge_external_index, poly))
		                  .face_index_map(CGAL::get(CGAL::face_external_index, poly));

		CGAL::Polygon_mesh_processing::isotropic_remeshing(input, data.get(a_targetEdgeLength), poly, params);

		result.push_back(mesh);
	}

	data.set(a_out, result);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_targetEdgeLength, "target_edge_length", 0.1f);
	meta.addAttribute(a_iterations, "iterations", 1u);
	meta.addAttribute(a_relaxationSteps, "relaxation_steps", 1u);
	meta.addAttribute(a_out, "out_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_targetEdgeLength, a_out);
	meta.addInfluence(a_iterations, a_out);
	meta.addInfluence(a_relaxationSteps, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/mesh_processing/isotropic_remeshing", init);
}  // namespace
