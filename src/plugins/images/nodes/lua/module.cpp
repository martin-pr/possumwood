#include <possumwood_sdk/node_implementation.h>

#include <lualib.h>

#include <lua/datatypes/state.h>
#include <lua/datatypes/context.h>

#include "lua/images.h"
#include "datatypes/pixmap.h"
#include "lua/opencv_image.h"

namespace {

struct Params {
	dependency_graph::InAttr<possumwood::lua::Context> a_inContext;
	dependency_graph::OutAttr<possumwood::lua::Context> a_outContext;
};

Params s_ldrParams;
Params s_hdrParams;
Params s_opencvParams;

template<typename PIXMAP, typename WRAPPER>
dependency_graph::State compute(dependency_graph::Values& data, const std::string& suffix, Params& params) {
	possumwood::lua::Context context = data.get(params.a_inContext);

	context.addModule(
		"images" + suffix,
		[suffix](possumwood::lua::State& state) {
			possumwood::images::init<PIXMAP, WRAPPER>(state, suffix);
		}
	);

	data.set(params.a_outContext, context);

	return dependency_graph::State();
}

template<typename PIXMAP, typename WRAPPER>
void init(possumwood::Metadata& meta, const std::string& suffix, Params& params) {
	meta.addAttribute(params.a_inContext, "in_context", possumwood::lua::Context(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(params.a_outContext, "out_context", possumwood::lua::Context(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(params.a_inContext, params.a_outContext);

	meta.setCompute([suffix, &params](dependency_graph::Values& data) -> dependency_graph::State {
		return compute<PIXMAP, WRAPPER>(data, suffix, params);
	});
}

possumwood::NodeImplementation s_impl_ldr("lua/modules/images", [](possumwood::Metadata& meta) {
	init<possumwood::LDRPixmap, possumwood::images::PixmapWrapper<possumwood::LDRPixmap>>(meta, "", s_ldrParams);
});

possumwood::NodeImplementation s_impl_hdr("lua/modules/images_hdr", [](possumwood::Metadata& meta) {
	init<possumwood::HDRPixmap, possumwood::images::PixmapWrapper<possumwood::HDRPixmap>>(meta, "_hdr", s_hdrParams);
});

possumwood::NodeImplementation s_impl_opencv("lua/modules/images_opencv", [](possumwood::Metadata& meta) {
	init<possumwood::images::OpencvMatWrapper, possumwood::images::OpencvMatWrapper>(meta, "_opencv", s_opencvParams);
});

}
