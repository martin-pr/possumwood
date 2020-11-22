#include "vec3.h"

namespace possumwood {

namespace lua {

Vec3::operator Imath::V3f() const {
	return Imath::V3f(x, y, z);
}

}  // namespace lua

}  // namespace possumwood
