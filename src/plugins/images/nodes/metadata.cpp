#include <memory>

#include <possumwood_sdk/node_implementation.h>

#include "datatypes/pixmap.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::Pixmap>> a_image;
dependency_graph::OutAttr<unsigned> a_width, a_height;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::shared_ptr<const possumwood::Pixmap> input = data.get(a_image);

	dependency_graph::State result;
	if(input == nullptr) {
		result.addError("No input pixmap.");
		data.set(a_width, 0u);
		data.set(a_height, 0u);
	}
	else {
		data.set(a_width, (unsigned)input->width());
		data.set(a_height, (unsigned)input->height());
	}

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_width, "width", 0u);
	meta.addAttribute(a_height, "height", 0u);

	meta.addAttribute(a_image, "image");

	meta.addInfluence(a_image, a_width);
	meta.addInfluence(a_image, a_height);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("images/metadata", init);

}
