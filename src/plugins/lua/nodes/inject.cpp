#include "inject.h"

namespace {

possumwood::lua::Inject<unsigned> s_unsignedInject;
possumwood::lua::Inject<float> s_floatInject;
possumwood::lua::Inject<bool> s_boolInject;

possumwood::NodeImplementation s_impl_unsigned("lua/inject/unsigned",
                                               [](possumwood::Metadata& meta) { s_unsignedInject.init(meta); });
possumwood::NodeImplementation s_impl_float("lua/inject/float",
                                            [](possumwood::Metadata& meta) { s_floatInject.init(meta); });
possumwood::NodeImplementation s_impl_bool("lua/inject/bool",
                                           [](possumwood::Metadata& meta) { s_boolInject.init(meta); });

}  // namespace
