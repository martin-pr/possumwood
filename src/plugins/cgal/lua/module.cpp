#include "module.h"

#include "polyhedron.h"

namespace possumwood {
namespace cgal {

std::string Module::name() {
	return "cgal";
}

void Module::init(possumwood::lua::State& state) {
	using namespace luabind;

	module(state, name().c_str())[class_<PolyhedronWrapper::Face>("face")
	                                  .def(luabind::constructor<>())
	                                  .def("addVertex", &PolyhedronWrapper::Face::addVertex),

	                              class_<PolyhedronWrapper>("mesh")
	                                  .def(luabind::constructor<std::string>())
	                                  .def("addPoint", (std::size_t(PolyhedronWrapper::*)(float, float, float)) &
	                                                       PolyhedronWrapper::addPoint)
	                                  .def("addPoint", (std::size_t(PolyhedronWrapper::*)(const lua::Vec3&)) &
	                                                       PolyhedronWrapper::addPoint)
	                                  .def("addFace", &PolyhedronWrapper::addFace)];
}

}  // namespace cgal
}  // namespace possumwood
