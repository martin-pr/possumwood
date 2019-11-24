#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/context.h>
#include <lua/datatypes/variable.inl>

#include "lua/images.h"
#include "lua/pixmap_wrapper.h"
#include "lua/opencv_image.h"

#include "lua/nodes/extract.h"

#include "opencv/frame.h"

namespace {

possumwood::NodeImplementation s_impl_ldr("lua/extract/image",
	possumwood::lua::Extract<
		std::shared_ptr<const possumwood::LDRPixmap>,
		possumwood::images::PixmapWrapper<possumwood::LDRPixmap>
	>::init
);

possumwood::NodeImplementation s_impl_hdr("lua/extract/image_hdr",
	possumwood::lua::Extract<
		std::shared_ptr<const possumwood::HDRPixmap>,
		possumwood::images::PixmapWrapper<possumwood::HDRPixmap>
	>::init
);

possumwood::NodeImplementation s_impl_opencv("lua/extract/opencv_image",
	possumwood::lua::Extract<
		possumwood::opencv::Frame,
		possumwood::images::OpencvMatWrapper
	>::init
);

}
