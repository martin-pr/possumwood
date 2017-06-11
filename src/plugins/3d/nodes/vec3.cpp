#include <possumwood_sdk/node_implementation.h>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <OpenEXR/ImathVec.h>

#include "io/vec3.h"

namespace {

dependency_graph::InAttr<float> a_x, a_y, a_z;
dependency_graph::OutAttr<Imath::Vec3<float>> a_out;

void compute(dependency_graph::Values& data) {
	const float x = data.get(a_x);
	const float y = data.get(a_y);
	const float z = data.get(a_z);

	data.set(a_out, Imath::Vec3<float>(x, y, z));
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_x, "x");
	meta.addAttribute(a_y, "y");
	meta.addAttribute(a_z, "z");
	meta.addAttribute(a_out, "out");

	meta.addInfluence(a_x, a_out);
	meta.addInfluence(a_y, a_out);
	meta.addInfluence(a_z, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("3d/make_vec3", init);

}
