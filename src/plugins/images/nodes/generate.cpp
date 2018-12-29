#include <memory>

#include <boost/filesystem.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "datatypes/pixmap.h"
#include "datatypes/rgb.h"

namespace {

dependency_graph::InAttr<unsigned> a_width, a_height;
dependency_graph::InAttr<QColor> a_colour;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::Pixmap>> a_image;

dependency_graph::State compute(dependency_graph::Values& data) {
	const QColor col = data.get(a_colour);

	possumwood::Pixmap::channel_t red = col.red();
	possumwood::Pixmap::channel_t green = col.green();
	possumwood::Pixmap::channel_t blue = col.blue();

	possumwood::Pixel defaultValue(possumwood::Pixel::value_t{{red, green, blue}});

	std::shared_ptr<possumwood::Pixmap> result(new possumwood::Pixmap(data.get(a_width), data.get(a_height), defaultValue));

	data.set(a_image, std::shared_ptr<const possumwood::Pixmap>(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_width, "width", 512u);
	meta.addAttribute(a_height, "height", 512u);
	meta.addAttribute(a_colour, "colour", QColor(0,0,0));

	meta.addAttribute(a_image, "image");

	meta.addInfluence(a_width, a_image);
	meta.addInfluence(a_height, a_image);
	meta.addInfluence(a_colour, a_image);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("images/generate", init);

}
