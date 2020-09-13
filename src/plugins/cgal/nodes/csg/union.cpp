#include <possumwood_sdk/node_implementation.h>

#include <OpenEXR/ImathEuler.h>
#include <OpenEXR/ImathVec.h>

#include "cgal.h"
#include "datatypes/meshes.h"
#include "maths/io/vec3.h"

namespace {

using possumwood::CGALPolyhedron;

dependency_graph::InAttr<possumwood::CGALNefPolyhedron> a_in1, a_in2;
dependency_graph::OutAttr<possumwood::CGALNefPolyhedron> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::unique_ptr<possumwood::CGALNef> nef(new possumwood::CGALNef(*data.get(a_in1)));
	(*nef) += *data.get(a_in2);

	data.set(a_out, possumwood::CGALNefPolyhedron(std::move(nef)));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in1, "nef_1", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_in2, "nef_2", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_out, "nef", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in1, a_out);
	meta.addInfluence(a_in2, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/csg/union", init);

}  // namespace
