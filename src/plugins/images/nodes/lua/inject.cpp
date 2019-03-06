#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/context.h>
#include <lua/datatypes/variable.inl>

#include "lua/images.h"
#include "lua/pixmap_wrapper.h"

#include "lua/nodes/inject.h"

namespace {

possumwood::NodeImplementation s_impl_ldr("lua/inject/image",
	possumwood::lua::Inject<std::shared_ptr<const possumwood::LDRPixmap>,
	possumwood::images::PixmapWrapper<possumwood::LDRPixmap>>::init
);

possumwood::NodeImplementation s_impl_hdr("lua/inject/image_hdr",
	possumwood::lua::Inject<std::shared_ptr<const possumwood::HDRPixmap>,
	possumwood::images::PixmapWrapper<possumwood::HDRPixmap>>::init
);

}
