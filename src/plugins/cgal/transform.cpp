#include "transform.h"

namespace possumwood {

namespace {

void transformNormals(possumwood::Property<std::array<float, 3>>& prop, const Imath::Matrix44<float>& normalMatrix) {
	for(auto& v : prop) {
		Imath::Vec4<float> p(v[0], v[1], v[2], 0.0f);
		p *= normalMatrix;
		v[0] = p.x;
		v[1] = p.y;
		v[2] = p.z;
	}
}

}  // namespace

Meshes transform(const Meshes& meshes, const Imath::Matrix44<float>& matrix, bool transformVec3PropsAsNormals) {
	Meshes out = meshes;

	const Imath::Matrix44<float> normalMatrix = matrix.inverse().transposed();

	Meshes result = meshes;
	for(auto& mesh_ : result) {
		auto& mesh = mesh_.edit();
		for(auto it = mesh.polyhedron().vertices_begin(); it != mesh.polyhedron().vertices_end(); ++it) {
			possumwood::CGALPolyhedron::Point& pt = it->point();

			Imath::Vec3<float> p(pt[0], pt[1], pt[2]);
			p *= matrix;

			pt = possumwood::CGALKernel::Point_3(p.x, p.y, p.z);
		}

		if(transformVec3PropsAsNormals) {
			for(auto& prop : mesh.faceProperties())
				if(prop.type() == typeid(std::array<float, 3>))
					transformNormals(mesh.faceProperties().property<std::array<float, 3>>(prop.name()), normalMatrix);

			for(auto& prop : mesh.halfedgeProperties())
				if(prop.type() == typeid(std::array<float, 3>))
					transformNormals(mesh.halfedgeProperties().property<std::array<float, 3>>(prop.name()),
					                 normalMatrix);

			for(auto& prop : mesh.vertexProperties())
				if(prop.type() == typeid(std::array<float, 3>))
					transformNormals(mesh.vertexProperties().property<std::array<float, 3>>(prop.name()), normalMatrix);
		}
	}

	return result;
}

}  // namespace possumwood
