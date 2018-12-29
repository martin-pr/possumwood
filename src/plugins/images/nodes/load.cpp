#include <memory>

#include <boost/filesystem.hpp>

#include <QPixmap>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "datatypes/pixmap.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::Pixmap>> a_image;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State out;

	const possumwood::Filename filename = data.get(a_filename);

	std::shared_ptr<possumwood::Pixmap> result;

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename())) {
		QPixmap pixmap(filename.filename().string().c_str());

		QImage image = pixmap.toImage();
		image = image.convertToFormat(QImage::Format_RGB888);
		assert(image.format() == QImage::Format_RGB888);

		if(!pixmap.isNull()) {
			result = std::shared_ptr<possumwood::Pixmap>(new possumwood::Pixmap(image.width(), image.height()));

			for(std::size_t y=0; y<(std::size_t)image.height(); ++y) {
				const uchar* scanline = image.constScanLine(y);

				for(std::size_t x=0; x<(std::size_t)image.width(); ++x)
					(*result)(x, y).setValue(possumwood::Pixel::value_t{{scanline[x*3], scanline[x*3+1], scanline[x*3+2]}});
			}
		}
	}

	data.set(a_image, std::shared_ptr<const possumwood::Pixmap>(result));

	if(!result)
		out.addError("Cannot load filename " + filename.filename().string());

	return out;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"All supported images (*.png *.jpg *.jpeg)",
		"PNG images (*.png)",
		"JPG images (*.jpg; *.jpeg)"
	}));
	meta.addAttribute(a_image, "image");

	meta.addInfluence(a_filename, a_image);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("images/load", init);

}
