#include <possumwood_sdk/node_implementation.h>

#include "cgal.h"
#include "datatypes/meshes.h"
#include "errors.h"

#include <CGAL/minkowski_sum_3.h>

namespace {

using possumwood::CGALPolyhedron;

dependency_graph::InAttr<possumwood::CGALNefPolyhedron> a_nef1, a_nef2;
dependency_graph::OutAttr<possumwood::CGALNefPolyhedron> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	// the algorithm implementation is not const correct, yay
	possumwood::CGALNef nef1 = *data.get(a_nef1);
	possumwood::CGALNef nef2 = *data.get(a_nef2);

	std::unique_ptr<possumwood::CGALNef> nef(new possumwood::CGALNef(CGAL::minkowski_sum_3(nef1, nef2)));

	data.set(a_out, possumwood::CGALNefPolyhedron(std::move(nef)));

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_nef1, "nef1", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_nef2, "nef2", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "nef", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_nef1, a_out);
	meta.addInfluence(a_nef2, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/csg/minkowski", init);
}  // namespace
