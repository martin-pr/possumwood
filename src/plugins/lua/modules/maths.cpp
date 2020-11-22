#include "maths.h"

#include <luabind/operator.hpp>

#include "wrappers/vec3.h"

namespace possumwood {
namespace lua {

std::string MathsModule::name() {
	return "maths";
}

void MathsModule::init(possumwood::lua::State& state) {
	using namespace luabind;

	module(state, name().c_str())[class_<Vec3>("vec3")
	                                  .def(luabind::constructor<float, float, float>())
	                                  .def_readwrite("x", &Vec3::x)
	                                  .def_readwrite("y", &Vec3::y)
	                                  .def_readwrite("z", &Vec3::z)
	                                  .def(const_self + const_self)
	                                  .def(const_self - const_self)
	                                  .def(const_self * other<float>())
	                                  .def(other<float>() * const_self)];
}

}  // namespace lua
}  // namespace possumwood
