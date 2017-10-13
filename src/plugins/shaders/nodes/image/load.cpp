#include <memory>

#include <boost/filesystem.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "datatypes/pixmap.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<std::shared_ptr<const QPixmap>> a_image;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State out;

	const possumwood::Filename filename = data.get(a_filename);

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename())) {
		std::unique_ptr<QPixmap> pixmap(new QPixmap(
			filename.filename().string().c_str()));

		if(!pixmap->isNull())
			data.set(a_image, std::shared_ptr<const QPixmap>(pixmap.release()));
	}
	else {
		data.set(a_image, std::shared_ptr<const QPixmap>());
		out.addError("Cannot load filename " + filename.filename().string());
	}

	return out;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({{
		"PNG images (*.png);;"
		"JPG images (*.jpg; *.jpeg)"
	}}));
	meta.addAttribute(a_image, "image");

	meta.addInfluence(a_filename, a_image);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("shaders/image/load", init);

}
