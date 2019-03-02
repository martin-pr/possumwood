#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/context.h>
#include <lua/datatypes/variable.inl>

#include "lua/images.h"
#include "lua/pixmap_wrapper.h"

namespace {

using namespace possumwood::images;

template<typename PIXMAP>
struct Params {
	dependency_graph::InAttr<std::string> a_name;
	dependency_graph::InAttr<std::shared_ptr<const possumwood::lua::State>> a_state;
	dependency_graph::OutAttr<std::shared_ptr<const PIXMAP>> a_out;
};

Params<possumwood::LDRPixmap> s_ldrParams;
Params<possumwood::HDRPixmap> s_hdrParams;

template<typename PIXMAP>
static dependency_graph::State compute(dependency_graph::Values& data, Params<PIXMAP>& params) {
	std::shared_ptr<const possumwood::lua::State> state = data.get(params.a_state);

	if(!state)
		throw std::runtime_error("Uninitialised state - cannot extract a value.");

	try {
		// get the result
		std::shared_ptr<PixmapWrapper<PIXMAP>> value =
			luabind::object_cast<std::shared_ptr<PixmapWrapper<PIXMAP>>>(state->globals()[data.get(params.a_name)]);
		// and push it to the output
		data.set(params.a_out, std::shared_ptr<const PIXMAP>(new PIXMAP(value->pixmap())));
	}
	catch(const luabind::error& err) {
		throw std::runtime_error(lua_tostring(err.state(), -1));
	}

	return dependency_graph::State();
}

template<typename PIXMAP>
void init(possumwood::Metadata& meta, Params<PIXMAP>& params) {
	meta.addAttribute(params.a_name, "name", std::string("variable"));
	meta.addAttribute(params.a_state, "state");

	meta.addAttribute(params.a_out, "out");

	meta.addInfluence(params.a_name, params.a_out);
	meta.addInfluence(params.a_state, params.a_out);

	meta.setCompute([&](dependency_graph::Values& data) {
		return compute<PIXMAP>(data, params);
	});
}

possumwood::NodeImplementation s_impl_ldr("lua/extract/image", [](possumwood::Metadata& meta) {
	init<possumwood::LDRPixmap>(meta, s_ldrParams);
});

possumwood::NodeImplementation s_impl_hdr("lua/extract/image_hdr", [](possumwood::Metadata& meta) {
	init<possumwood::HDRPixmap>(meta, s_hdrParams);
});

}
