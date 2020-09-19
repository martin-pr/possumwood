#include "lua/nodes/extract.h"

#include <lua/datatypes/context.h>
#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/variable.inl>

#include "lua/opencv_image.h"
#include "opencv/frame.h"

namespace {

possumwood::NodeImplementation s_impl("lua/extract/image",
                                      possumwood::lua::Extract<possumwood::opencv::Frame,
                                                               possumwood::images::OpencvMatWrapper,
                                                               possumwood::AttrFlags::kVertical>::init);

}
