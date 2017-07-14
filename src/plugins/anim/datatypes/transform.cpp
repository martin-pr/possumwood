#include "transform.h"

#include <ImathMatrixAlgo.h>

namespace anim {

Transform::Transform() :
	// vector has to be initialised explicitly, but quaternion doesn't
	translation(0,0,0), rotation(1,0,0,0) {
}

Transform::Transform(const Imath::V3f& tr) : translation(tr), rotation(1,0,0,0) {
}

Transform::Transform(const Imath::Quatf& rot, const Imath::V3f& tr) :
	translation(tr), rotation(rot) {

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

std::ostream& operator << (std::ostream& out, const Transform& tr) {
	out << "(" << tr.rotation << "), (" << tr.translation << ")";

	return out;
}

};
