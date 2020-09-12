#include <unordered_map>

#include <CGAL/Nef_polyhedron_3.h>

#include <possumwood_sdk/node_implementation.h>

#include "cgal.h"
#include "convert.h"
#include "datatypes/meshes.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

dependency_graph::InAttr<possumwood::CGALNefPolyhedron> a_nef;
dependency_graph::OutAttr<Meshes> a_polymesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State state;

	possumwood::ScopedOutputRedirect outRedirect;

	auto nef = data.get(a_nef);

	if(!(*nef).is_simple()) {
		throw std::runtime_error(
		    "Nef to polymesh conversion failed - Nef polyhedron does not represent a closed surface.");
	}

	Meshes result;

	try {
		CGAL::Polyhedron_3<possumwood::NefKernel> poly;
		(*nef).convert_to_polyhedron(poly);

		CGALPolyhedron out;
		possumwood::convert(poly, out);

		result.addMesh("polyhedron");
		result.begin()->edit().polyhedron() = out;

		// any warnings / errors that were not converted to exceptions by CGAL
		state.append(outRedirect.state());
	}
	catch(const std::exception& err) {
		// usually, the output printouts in CGAL are better than the exceptions - let's use those first
		state.append(outRedirect.state());
		// and add the exception content
		state.addError(err.what());
	}

	data.set(a_polymesh, result);

	return state;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_nef, "nef", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_polymesh, "mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_nef, a_polymesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/csg/nef_to_poly", init);
}  // namespace
