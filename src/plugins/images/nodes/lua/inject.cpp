#include "lua/nodes/inject.h"

#include <lua/datatypes/context.h>
#include <lua/datatypes/state.h>
#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/variable.inl>
#include <luabind/operator.hpp>

#include "lua/module.h"
#include "lua/opencv_image.h"
#include "opencv/frame.h"

namespace possumwood {
namespace images {

namespace {

possumwood::lua::
    Inject<possumwood::opencv::Frame, possumwood::images::OpencvMatWrapper, Module, possumwood::AttrFlags::kVertical>
        s_injector;

possumwood::NodeImplementation s_impl("lua/inject/image", [](possumwood::Metadata& meta) { s_injector.init(meta); });

}  // namespace

}  // namespace images
}  // namespace possumwood
