#pragma once

#include <ImathQuat.h>
#include <ImathVec.h>

#include <iostream>

namespace anim {

/// a single rigid body transformation
struct Transform {
	Imath::V3f translation;
	Imath::Quatf rotation;

	/// initialises the transformation to identity
	Transform();
	/// initialises only translation part
	Transform(const Imath::V3f& tr);
	/// initialises both translation and rotation
	Transform(const Imath::Quatf& rot, const Imath::V3f& tr = Imath::V3f(0, 0, 0));
	/// convert a rigid body transformation matrix to a Transform object
	Transform(const Imath::M44f& m);

	const Imath::M44f toMatrix44() const;

	const Transform operator*(const Transform& t) const;
	Transform& operator*=(const Transform& t);

	const Transform operator*(const Imath::Matrix44<float>& m) const;
	Transform& operator*=(const Imath::Matrix44<float>& m);

	void invert();
	Transform inverse() const;

	bool operator==(const Transform& t) const;
	bool operator!=(const Transform& t) const;
};

std::ostream& operator<<(std::ostream& out, const Transform& tr);

};  // namespace anim
