#include <possumwood_sdk/node_implementation.h>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathEuler.h>

#include "maths/io/vec3.h"
#include "datatypes/mesh.h"
#include "openmesh.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const Mesh>> a_inMesh;
dependency_graph::InAttr<Imath::Vec3<float>> a_translation, a_rotation, a_scale;
dependency_graph::OutAttr<std::shared_ptr<const Mesh>> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	const std::shared_ptr<const Mesh> mesh = data.get(a_inMesh);
	const Imath::Vec3<float> tr = data.get(a_translation);
	const Imath::Vec3<float> rot = data.get(a_rotation);
	const Imath::Vec3<float> sc = data.get(a_scale);

	if(mesh) {
		std::unique_ptr<Mesh> newMesh(new Mesh(*mesh));

		Imath::Matrix44<float> m1, m2, m3;
		m1 = Imath::Euler<float>(Imath::Vec3<float>(
			rot.x * M_PI / 180.0,
			rot.y * M_PI / 180.0,
			rot.z * M_PI / 180.0
		)).toMatrix44();
		m2.setScale(sc);
		m3.setTranslation(tr);

		const Imath::Matrix44<float> matrix = m1 * m2 * m3;

		for(auto it = newMesh->vertices_begin(); it != newMesh->vertices_end(); ++it) {
			Mesh::Point& pt = newMesh->point(*it);

			Imath::Vec3<float> p(pt[0], pt[1], pt[2]);
			p *= matrix;

			pt[0] = p[0];
			pt[1] = p[1];
			pt[2] = p[2];
		}

		data.set(a_outMesh, std::shared_ptr<const Mesh>(newMesh.release()));
	}
	else
		data.set(a_outMesh, std::shared_ptr<const Mesh>());

	return dependency_graph::State();

}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "in_mesh");
	meta.addAttribute(a_translation, "translation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_rotation, "rotation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_scale, "scale", Imath::Vec3<float>(1, 1, 1));
	meta.addAttribute(a_outMesh, "out_mesh");

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_translation, a_outMesh);
	meta.addInfluence(a_rotation, a_outMesh);
	meta.addInfluence(a_scale, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("openmesh/transform", init);

}
