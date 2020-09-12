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

dependency_graph::InAttr<Meshes> a_polymesh;
dependency_graph::OutAttr<possumwood::CGALNefPolyhedron> a_nef;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State state;

	possumwood::ScopedOutputRedirect outRedirect;

	try {
		possumwood::CGALNef result;

		for(auto& mesh : data.get(a_polymesh)) {
			// convert the data to a compatible kernel type
			CGAL::Polyhedron_3<possumwood::NefKernel> poly;

			possumwood::convert(mesh.polyhedron(), poly);

			if(!poly.is_closed())
				throw std::runtime_error("Polymesh " + mesh.name() + " is not closed!");

			result = possumwood::CGALNef(poly);
		}

		data.set(a_nef,
		         possumwood::CGALNefPolyhedron(std::unique_ptr<possumwood::CGALNef>(new possumwood::CGALNef(result))));

		// any warnings / errors that were not converted to exceptions by CGAL
		state.append(outRedirect.state());
	} catch(const std::exception& err) {
		// usually, the output printouts in CGAL are better than the exceptions - let's use those first
		state.append(outRedirect.state());
		// and add the exception content
		state.addError(err.what());
	}

	return state;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_polymesh, "mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_nef, "nef", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_polymesh, a_nef);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/csg/poly_to_nef", init);
}  // namespace
