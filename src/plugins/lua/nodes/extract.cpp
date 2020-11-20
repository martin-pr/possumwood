#include "extract.h"

#include "wrappers/vec3.h"

#include "maths/io/vec3.h"

namespace {

possumwood::NodeImplementation s_impl_unsigned("lua/extract/unsigned", possumwood::lua::Extract<unsigned>::init);

possumwood::NodeImplementation s_impl_float("lua/extract/float", possumwood::lua::Extract<float>::init);

possumwood::NodeImplementation s_impl_vec3("lua/extract/vec3",
                                           possumwood::lua::Extract<Imath::V3f, possumwood::lua::Vec3>::init);

}  // namespace
