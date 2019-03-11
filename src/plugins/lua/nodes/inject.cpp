#include "inject.h"

namespace {

possumwood::NodeImplementation s_impl_unsigned("lua/inject/unsigned",
	possumwood::lua::Inject<unsigned>::init);

possumwood::NodeImplementation s_impl_float("lua/inject/float",
	possumwood::lua::Inject<float>::init);

}
