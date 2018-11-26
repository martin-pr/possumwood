#include <memory>

#include <boost/filesystem.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "datatypes/pixmap.h"
#include "datatypes/rgb.h"

namespace {

dependency_graph::InAttr<unsigned> a_width, a_height;
dependency_graph::InAttr<QColor> a_colour;
dependency_graph::OutAttr<std::shared_ptr<const QPixmap>> a_image;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::shared_ptr<QPixmap> result(new QPixmap(data.get(a_width), data.get(a_height)));
	result->fill(data.get(a_colour));

	data.set(a_image, std::shared_ptr<const QPixmap>(result));

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
