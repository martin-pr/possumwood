#include "transform.h"

#include <ImathMatrixAlgo.h>

#include <cmath>

namespace anim {

Transform::Transform() :
	// vector has to be initialised explicitly, but quaternion doesn't
	translation(0, 0, 0), rotation(1, 0, 0, 0) {
}

Transform::Transform(const Imath::V3f& tr) : translation(tr), rotation(1, 0, 0, 0) {
}

Transform::Transform(const Imath::Quatf& rot, const Imath::V3f& tr) :
	translation(tr), rotation(rot) {

	// the quaternion should always be a unit quat to represent rotation. Let's re-normalize, to avoid accumulation
	//   of floating point errors
	rotation.normalize();
}

Transform::Transform(const Imath::M44f& m) {
	rotation = ~Imath::extractQuat(m);
	translation[0] = m[0][3];
	translation[1] = m[1][3];
	translation[2] = m[2][3];
}

const Imath::M44f Transform::toMatrix44() const {
	Imath::M44f result = rotation.toMatrix44().transpose();
	result[0][3] = translation[0];
	result[1][3] = translation[1];
	result[2][3] = translation[2];

	return result;
}

const Transform Transform::operator * (const Transform& t) const {
	return Transform(
	           rotation * t.rotation,
	           translation + t.translation * rotation
	       );
}

Transform& Transform::operator *= (const Transform& t) {
	translation = translation + t.translation * rotation;
	rotation = rotation * t.rotation;

	return *this;
}

const Transform Transform::operator * (const Imath::Matrix44<float>& m) const {
	Transform result = *this;
	result *= m;
	return result;
}

Transform& Transform::operator *= (const Imath::Matrix44<float>& _m) {
	Imath::Matrix44<float> m = _m;

	translation = translation * m;

	// extract translation
	const Imath::Vec3<float> tr(m[3][0], m[3][1], m[3][2]);

	// extract scale
	Imath::Vec3<float> sc;
	for(unsigned a = 0; a < 3; ++a)
		sc[a] = std::sqrt(powf(m[0][a], 2) + powf(m[1][a], 2) + powf(m[2][a], 2));

	// undo scale from the matrix
	for(unsigned a=0;a<3;++a)
		for(unsigned b=0;b<3;++b)
			m[a][b] /= sc[b];

	// and apply this all (ARGH, this is the wrong way around! OpenEXR is a bit inconsistent)
	rotation = Imath::extractQuat(m) * rotation;

	return *this;
}

void Transform::invert() {
	// invert the translation (with rotation correction)
	translation = -translation * ~rotation;
	// quaternion conjugate
	rotation.v = -rotation.v;
}

Transform Transform::inverse() const {
	Transform result = *this;
	result.invert();
	return result;
}

bool Transform::operator == (const Transform& t) const {
	return translation == t.translation && rotation == t.rotation;
}

bool Transform::operator != (const Transform& t) const {
	return translation != t.translation || rotation != t.rotation;
}

std::ostream& operator << (std::ostream& out, const Transform& tr) {
	out << "(" << tr.rotation << "), (" << tr.translation << ")";

	return out;
}

};
