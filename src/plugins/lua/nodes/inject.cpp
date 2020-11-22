#include "inject.h"

#include "modules/maths.h"
#include "wrappers/vec3.h"

#include "maths/io/vec3.h"

namespace {

possumwood::lua::Inject<unsigned> s_unsignedInject;
possumwood::lua::Inject<float> s_floatInject;
possumwood::lua::Inject<bool> s_boolInject;
possumwood::lua::Inject<Imath::V3f, possumwood::lua::Vec3, possumwood::lua::MathsModule> s_vec3Inject;

possumwood::NodeImplementation s_impl_unsigned("lua/inject/unsigned",
                                               [](possumwood::Metadata& meta) { s_unsignedInject.init(meta); });
possumwood::NodeImplementation s_impl_float("lua/inject/float",
                                            [](possumwood::Metadata& meta) { s_floatInject.init(meta); });
possumwood::NodeImplementation s_impl_bool("lua/inject/bool",
                                           [](possumwood::Metadata& meta) { s_boolInject.init(meta); });
possumwood::NodeImplementation s_impl_vec3("lua/inject/vec3",
                                           [](possumwood::Metadata& meta) { s_vec3Inject.init(meta); });

}  // namespace
