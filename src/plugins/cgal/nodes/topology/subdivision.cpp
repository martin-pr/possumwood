#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <CGAL/subdivision_method_3.h>

#include "cgal.h"
#include "datatypes/meshes.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

dependency_graph::InAttr<Meshes> a_in;
dependency_graph::InAttr<unsigned> a_level;
dependency_graph::InAttr<possumwood::Enum> a_method;
dependency_graph::OutAttr<Meshes> a_out;

enum Method { kCatmullClark, kLoop, kDooSabin, kSqrt3 };

static std::vector<std::pair<std::string, int>> s_method{
    {"Catmull Clark", kCatmullClark},
    {"Loop", kLoop},
    {"Doo Sabin", kDooSabin},
    {"Sqrt 3", kSqrt3},
};

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	Meshes result = data.get(a_in);

	for(auto& mesh : result) {
		auto& poly = mesh.edit();

		auto params = CGAL::parameters::number_of_iterations(data.get(a_level))
		                  .vertex_index_map(CGAL::get(CGAL::vertex_external_index, poly.polyhedron()))
		                  .halfedge_index_map(CGAL::get(CGAL::halfedge_external_index, poly.polyhedron()))
		                  .face_index_map(CGAL::get(CGAL::halfedge_external_index, poly.polyhedron()));

		switch(data.get(a_method).intValue()) {
			case kCatmullClark:
				CGAL::Subdivision_method_3::CatmullClark_subdivision(poly.polyhedron(), params);
				break;

			case kLoop:
				CGAL::Subdivision_method_3::Loop_subdivision(poly.polyhedron(), params);
				break;

			case kDooSabin:
				CGAL::Subdivision_method_3::DooSabin_subdivision(poly.polyhedron(), params);
				break;

			case kSqrt3:
				CGAL::Subdivision_method_3::Sqrt3_subdivision(poly.polyhedron(), params);
				break;
		}
	}

	data.set(a_out, result);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_level, "level", 2u);
	meta.addAttribute(a_method, "method", possumwood::Enum(s_method.begin(), s_method.end()));
	meta.addAttribute(a_out, "out_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_level, a_out);
	meta.addInfluence(a_method, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/topology/subdivision", init);
}  // namespace
