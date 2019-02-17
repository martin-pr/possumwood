#include <memory>

#include <boost/filesystem.hpp>

#include <OpenImageIO/imageio.h>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include "datatypes/pixmap.h"

namespace {

using namespace OIIO;

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::Pixmap>> a_image;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State out;

	const possumwood::Filename filename = data.get(a_filename);

	std::shared_ptr<possumwood::Pixmap> result;

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename())) {
		auto in = ImageInput::open(filename.filename().string());
		if (!in)
			throw std::runtime_error("Error loading " + filename.filename().string());

		const ImageSpec &spec = in->spec();
		int xres = spec.width;
		int yres = spec.height;
		int channels = spec.nchannels;
		if(spec.nchannels != 3)
			throw std::runtime_error("Error loading " + filename.filename().string() + " - only images with 3 channels are supported at the moment!");

		std::vector<unsigned char> pixels(xres*yres*channels);
		in->read_image(TypeDesc::UINT8, &pixels[0]);
		in->close();

		result = std::shared_ptr<possumwood::Pixmap>(new possumwood::Pixmap(xres, yres));

		for(int y=0; y < yres; ++y) {
			for(int x=0; x<xres; ++x)
				(*result)(x, y).setValue(possumwood::Pixel::value_t{{
					pixels[(x+y*xres)*3],
					pixels[(x+y*xres)*3+1],
					pixels[(x+y*xres)*3+2]
				}});
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
