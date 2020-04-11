#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <tbb/parallel_for.h>

#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>
#include <OpenImageIO/imageio.h>

#include <actions/traits.h>

#include "frame.h"
#include "exif.h"

namespace {

using namespace OIIO;

template<typename T>
struct ImageTraits;

template<>
struct ImageTraits<unsigned char> {
	static const TypeDesc::BASETYPE oiio_type = TypeDesc::UINT8;
	static const int opencv_type = CV_8U;
};

template<>
struct ImageTraits<float> {
	static const TypeDesc::BASETYPE oiio_type = TypeDesc::FLOAT;
	static const int opencv_type = CV_32F;
};

template<typename T>
void copyData(ImageInput& input, cv::Mat& m) {
	const ImageSpec &spec = input.spec();
	std::size_t xres = spec.width;
	std::size_t yres = spec.height;
	std::size_t channels = spec.nchannels;

	std::vector<T> pixels(xres*yres*channels);
	input.read_image(ImageTraits<T>::oiio_type, pixels.data());
	input.close();

	m = cv::Mat::zeros(yres, xres, CV_MAKETYPE(ImageTraits<T>::opencv_type, channels));

	tbb::parallel_for(std::size_t(0), yres, [&](std::size_t y) {
		for(std::size_t x=0; x < xres; ++x) {
			T* ptr = m.ptr<T>(y, x);

			for(std::size_t c=0; c < channels; ++c) {
				// reverse channels - oiio RGB to opencv BGR
				std::size_t index = channels - c - 1;

				T value = *(pixels.data() + (y*xres+x)*channels + c);
				assert(value > 0 || value == 0);
				*(ptr + index) = value;
			}
		}
	});
}

/////////////////////////////

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::OutAttr<possumwood::opencv::Exif> a_exif;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	// native reader - cannot read or understand EXIF information
	// data.set(a_frame, possumwood::opencv::Frame(cv::imread(filename.filename().string())));

	cv::Mat result;
	possumwood::opencv::Exif exif;

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename())) {
		std::unique_ptr<ImageInput> in(ImageInput::open(filename.filename().string()));
		if (!in)
			throw std::runtime_error("Error loading " + filename.filename().string());

		// get the image spec
		const ImageSpec &spec = in->spec();

		// convert the raw data to a cv::Mat type
		if(spec.format == TypeDesc::UINT8)
			copyData<unsigned char>(*in, result);
		else if(spec.format == TypeDesc::FLOAT)
			copyData<float>(*in, result);
		else
			throw std::runtime_error("Error loading " + filename.filename().string() + " - only images with 8 or 32 bits per channel are supported at the moment!");

		// process metadata (EXIF)
		float exposure = 0.0f;
		{
			auto it = spec.extra_attribs.find("ExposureTime");
			if(it != spec.extra_attribs.end())
				// exposure = (*static_cast<const float*>(it->data()));
				exposure = it->get_float();
		}

		float fnumber = 0.0f;
		{
			auto it = spec.extra_attribs.find("FNumber");
			if(it != spec.extra_attribs.end())
				fnumber = it->get_float();
		}

		float iso = 100.0f;
		{
			auto it = spec.extra_attribs.find("Exif:PhotographicSensitivity");
			if(it != spec.extra_attribs.end())
				iso = it->get_float();
		}

		// for(auto& a : spec.extra_attribs)
		// 	std::cout << a.name() << "  " << a.get_float() << std::endl;

		exif = possumwood::opencv::Exif(exposure, fnumber, iso);
		data.set(a_exif, exif);
	}

	data.set(a_frame, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"Image files (*.png *.jpg *.jpe *.jpeg *.exr *.tif *.tiff)",
	}));
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_exif, "exif", possumwood::opencv::Exif(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_filename, a_frame);
	meta.addInfluence(a_filename, a_exif);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/image", init);

}
