#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/pixmap.h"
#include "maths/io/vec2.h"

namespace {

template<typename PIXMAP>
struct Params {
	dependency_graph::InAttr<std::shared_ptr<const PIXMAP>> a_image;
	dependency_graph::OutAttr<Imath::Vec2<unsigned>> a_size;
};

Params<possumwood::LDRPixmap> s_ldrParams;
Params<possumwood::HDRPixmap> s_hdrParams;

template<typename PIXMAP>
dependency_graph::State compute(dependency_graph::Values& data, Params<PIXMAP>& params) {
	std::shared_ptr<const PIXMAP> input = data.get(params.a_image);

	dependency_graph::State result;
	if(input == nullptr) {
		result.addError("No input pixmap.");
		data.set(params.a_size, Imath::Vec2<unsigned>(0u, 0u));
	}
	else
		data.set(params.a_size, Imath::Vec2<unsigned>(input->width(), input->height()));

	return dependency_graph::State();
}

template<typename PIXMAP>
void init(possumwood::Metadata& meta, Params<PIXMAP>& params) {
	meta.addAttribute(params.a_size, "size", Imath::Vec2<unsigned>(0,0));
	meta.addAttribute(params.a_image, "image");

	meta.addInfluence(params.a_image, params.a_size);

	meta.setCompute([&params](dependency_graph::Values& data) {
		return compute<PIXMAP>(data, params);
	});
}

possumwood::NodeImplementation s_impl("images/metadata", [](possumwood::Metadata& meta) {
	init<possumwood::LDRPixmap>(meta, s_ldrParams);
});

possumwood::NodeImplementation s_impl_hdr("images/metadata_hdr", [](possumwood::Metadata& meta) {
	init<possumwood::HDRPixmap>(meta, s_hdrParams);
});

}
