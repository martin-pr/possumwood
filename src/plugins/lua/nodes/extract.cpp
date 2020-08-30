#include "extract.h"

namespace {

possumwood::NodeImplementation s_impl_unsigned("lua/extract/unsigned", possumwood::lua::Extract<unsigned>::init);

possumwood::NodeImplementation s_impl_float("lua/extract/float", possumwood::lua::Extract<float>::init);

}  // namespace
