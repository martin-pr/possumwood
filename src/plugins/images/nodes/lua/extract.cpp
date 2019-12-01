#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/context.h>
#include <lua/datatypes/variable.inl>

#include "lua/opencv_image.h"

#include "lua/nodes/extract.h"

#include "opencv/frame.h"

namespace {

possumwood::NodeImplementation s_impl_opencv("lua/extract/opencv_image",
	possumwood::lua::Extract<
		possumwood::opencv::Frame,
		possumwood::images::OpencvMatWrapper
	>::init
);

}
