#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/context.h>
#include <lua/datatypes/variable.inl>
#include <lua/datatypes/state.h>

#include "lua/opencv_image.h"
#include "lua/module.h"

#include "lua/nodes/inject.h"

#include "opencv/frame.h"

#include <luabind/operator.hpp>

namespace possumwood { namespace images {

namespace {

possumwood::lua::Inject<
	possumwood::opencv::Frame,
	possumwood::images::OpencvMatWrapper,
	Module
> s_injector;

possumwood::NodeImplementation s_impl_opencv("lua/inject/opencv_image",
	[](possumwood::Metadata& meta) {
		s_injector.init(meta);
	}
);

}

} }
