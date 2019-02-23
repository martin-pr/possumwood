#include <memory>

#include <boost/filesystem.hpp>

#include <ImfRgbaFile.h>
#include <ImfArray.h>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "datatypes/pixmap.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::HDRPixmap>> a_image;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State out;

	const possumwood::Filename filename = data.get(a_filename);

	std::shared_ptr<possumwood::HDRPixmap> result;

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename())) {

		Imf::RgbaInputFile file (filename.filename().string().c_str());
		Imath::Box2i dw = file.dataWindow();

		int width = dw.max.x - dw.min.x + 1;
		int height = dw.max.y - dw.min.y + 1;

		Imf::Array2D<Imf::Rgba> pixels;
		pixels.resizeErase (height, width);

		file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
		file.readPixels (dw.min.y, dw.max.y);

		result = std::shared_ptr<possumwood::HDRPixmap>(new possumwood::HDRPixmap(width, height));
		for(int y=0; y<height; ++y) {
			for(int x=0; x<width; ++x) {
				const Imf::Rgba& rgba = pixels[y][x];

				(*result)(x, y).setValue(possumwood::HDRPixmap::pixel_t::value_t{{rgba.r, rgba.g, rgba.b}});
			}
		}
	}

	data.set(a_image, std::shared_ptr<const possumwood::HDRPixmap>(result));

	if(!result)
		out.addError("Cannot load filename " + filename.filename().string());

	return out;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"OpenEXR images (*.exr)",
	}));
	meta.addAttribute(a_image, "image");

	meta.addInfluence(a_filename, a_image);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("images/load_hdr", init);

}
