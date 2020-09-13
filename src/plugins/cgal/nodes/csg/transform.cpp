#include <possumwood_sdk/node_implementation.h>

#include <OpenEXR/ImathEuler.h>
#include <OpenEXR/ImathVec.h>

#include "cgal.h"
#include "datatypes/meshes.h"
#include "maths/io/vec3.h"

namespace {

using possumwood::CGALPolyhedron;

dependency_graph::InAttr<possumwood::CGALNefPolyhedron> a_inMesh;
dependency_graph::InAttr<Imath::Vec3<float>> a_translation, a_rotation, a_scale;
dependency_graph::OutAttr<possumwood::CGALNefPolyhedron> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Imath::Vec3<float> tr = data.get(a_translation);
	const Imath::Vec3<float> rot = data.get(a_rotation);
	const Imath::Vec3<float> sc = data.get(a_scale);

	Imath::Matrix44<float> m1, m2, m3;
	m1 = Imath::Euler<float>(Imath::Vec3<float>(rot.x * M_PI / 180.0, rot.y * M_PI / 180.0, rot.z * M_PI / 180.0))
	         .toMatrix44();
	m2.setScale(sc);
	m3.setTranslation(tr);

	const Imath::Matrix44<float> m = m1 * m2 * m3;

	possumwood::CGALNef::Aff_transformation_3 transform(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1],
	                                                    m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[3][3]);

	std::unique_ptr<possumwood::CGALNef> nef(new possumwood::CGALNef(*data.get(a_inMesh)));
	nef->transform(transform);

	data.set(a_outMesh, possumwood::CGALNefPolyhedron(std::move(nef)));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "in_mesh", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_translation, "translation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_rotation, "rotation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_scale, "scale", Imath::Vec3<float>(1, 1, 1));
	meta.addAttribute(a_outMesh, "out_mesh", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_translation, a_outMesh);
	meta.addInfluence(a_rotation, a_outMesh);
	meta.addInfluence(a_scale, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/csg/transform", init);
}  // namespace
