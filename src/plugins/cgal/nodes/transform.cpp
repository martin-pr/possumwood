#include <OpenEXR/ImathEuler.h>
#include <OpenEXR/ImathVec.h>
#include <possumwood_sdk/node_implementation.h>

#include "cgal.h"
#include "datatypes/meshes.h"
#include "maths/io/vec3.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

dependency_graph::InAttr<Meshes> a_inMesh;
dependency_graph::InAttr<Imath::Vec3<float>> a_translation, a_rotation, a_scale;
dependency_graph::InAttr<bool> a_vec3AsNormal;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Imath::Vec3<float> tr = data.get(a_translation);
	const Imath::Vec3<float> rot = data.get(a_rotation);
	const Imath::Vec3<float> sc = data.get(a_scale);

	Imath::Matrix44<float> m1, m2, m3;
	m1 = Imath::Euler<float>(Imath::Vec3<float>(rot.x * M_PI / 180.0, rot.y * M_PI / 180.0, rot.z * M_PI / 180.0))
	         .toMatrix44();
	m2.setScale(sc);
	m3.setTranslation(tr);

	const Imath::Matrix44<float> matrix = m1 * m2 * m3;
	const Imath::Matrix44<float> normalMatrix = matrix.inverse().transposed();

	Meshes result = data.get(a_inMesh);
	for(auto& mesh_ : result) {
		auto& mesh = mesh_.edit();
		for(auto it = mesh.polyhedron().vertices_begin(); it != mesh.polyhedron().vertices_end(); ++it) {
			possumwood::CGALPolyhedron::Point& pt = it->point();

			Imath::Vec3<float> p(pt[0], pt[1], pt[2]);
			p *= matrix;

			pt = possumwood::CGALKernel::Point_3(p.x, p.y, p.z);
		}

		if(data.get(a_vec3AsNormal)) {
			for(auto& prop : mesh.faceProperties())
				if(prop.type() == typeid(std::array<float, 3>)) {
					for(auto& v : mesh.faceProperties().property<std::array<float, 3>>(prop.name())) {
						Imath::Vec4<float> p(v[0], v[1], v[2], 0.0f);
						p *= normalMatrix;
						v[0] = p.x;
						v[1] = p.y;
						v[2] = p.z;
					}
				}

			for(auto& prop : mesh.halfedgeProperties())
				if(prop.type() == typeid(std::array<float, 3>)) {
					for(auto& v : mesh.halfedgeProperties().property<std::array<float, 3>>(prop.name())) {
						Imath::Vec4<float> p(v[0], v[1], v[2], 0.0f);
						p *= normalMatrix;
						v[0] = p.x;
						v[1] = p.y;
						v[2] = p.z;
					}
				}

			for(auto& prop : mesh.vertexProperties())
				if(prop.type() == typeid(std::array<float, 3>)) {
					for(auto& v : mesh.vertexProperties().property<std::array<float, 3>>(prop.name())) {
						Imath::Vec4<float> p(v[0], v[1], v[2], 0.0f);
						p *= normalMatrix;
						v[0] = p.x;
						v[1] = p.y;
						v[2] = p.z;
					}
				}
		}
	}

	data.set(a_outMesh, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "in_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_translation, "translation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_rotation, "rotation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_scale, "scale", Imath::Vec3<float>(1, 1, 1));
	meta.addAttribute(a_vec3AsNormal, "vec3_as_normals", true);
	meta.addAttribute(a_outMesh, "out_mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_translation, a_outMesh);
	meta.addInfluence(a_rotation, a_outMesh);
	meta.addInfluence(a_scale, a_outMesh);
	meta.addInfluence(a_vec3AsNormal, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/transform", init);
}  // namespace
