#include "module.h"

#include "opencv_image.h"

namespace possumwood { namespace images {

std::string Module::name() {
	return "images";
}

void Module::init(possumwood::lua::State& state) {
	using namespace luabind;

	module(state, name().c_str())
	[
		class_<OpencvMatWrapper>("image")
			.enum_("types")
			[
				value("uint8", CV_8U),
				value("float32", CV_32F)
			]

			.def(luabind::constructor<std::size_t, std::size_t, int>())
			.def("setPixel", &OpencvMatWrapper::setPixel)
			.def("pixel", &OpencvMatWrapper::pixel)
			.def("width", &OpencvMatWrapper::width)
			.def("height", &OpencvMatWrapper::height)
	];
}

} }
