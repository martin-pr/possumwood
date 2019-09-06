#include <memory>

#include <boost/filesystem.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "maths/io/vec2u.h"
#include "datatypes/pixmap.h"
#include "datatypes/rgb.h"

namespace {

template<typename PIXMAP>
struct Params {
	dependency_graph::InAttr<Imath::Vec2<unsigned>> a_size;
	dependency_graph::InAttr<QColor> a_colour;
	dependency_graph::OutAttr<std::shared_ptr<const PIXMAP>> a_image;
};

Params<possumwood::LDRPixmap> s_ldrParams;
Params<possumwood::HDRPixmap> s_hdrParams;

void extract(possumwood::LDRPixel::value_t& pixel, const QColor& color) {
	pixel[0] = color.red();
	pixel[1] = color.green();
	pixel[2] = color.blue();
}

void extract(possumwood::HDRPixel::value_t& pixel, const QColor& color) {
	pixel[0] = color.redF();
	pixel[1] = color.greenF();
	pixel[2] = color.blueF();
}

template<typename PIXMAP>
dependency_graph::State compute(dependency_graph::Values& data, Params<PIXMAP>& params) {
	const QColor col = data.get(params.a_colour);

	typename PIXMAP::pixel_t::value_t value;
	extract(value, col);

	std::shared_ptr<PIXMAP> result(new PIXMAP(data.get(params.a_size)[0], data.get(params.a_size)[1], value));

	data.set(params.a_image, std::shared_ptr<const PIXMAP>(result));

	return dependency_graph::State();
}


template<typename PIXMAP>
void init(possumwood::Metadata& meta, Params<PIXMAP>& params) {
	meta.addAttribute(params.a_size, "size", Imath::Vec2<unsigned>(512u, 512u));
	meta.addAttribute(params.a_colour, "colour", QColor(0,0,0));

	meta.addAttribute(params.a_image, "image");

	meta.addInfluence(params.a_size, params.a_image);
	meta.addInfluence(params.a_colour, params.a_image);

	meta.setCompute([&params](dependency_graph::Values& data) {
		return compute<PIXMAP>(data, params);
	});
}

possumwood::NodeImplementation s_impl("images/generate", [](possumwood::Metadata& meta) {
	init<possumwood::LDRPixmap>(meta, s_ldrParams);
});

possumwood::NodeImplementation s_impl_hdr("images/generate_hdr", [](possumwood::Metadata& meta) {
	init<possumwood::HDRPixmap>(meta, s_hdrParams);
});

}
