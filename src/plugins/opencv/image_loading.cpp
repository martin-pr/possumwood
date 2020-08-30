#include "image_loading.h"

#include <OpenImageIO/imageio.h>
#include <tbb/parallel_for.h>

namespace possumwood {
namespace opencv {

using namespace OIIO;

namespace {

template <typename T>
struct ImageTraits;

template <>
struct ImageTraits<unsigned char> {
	static const TypeDesc::BASETYPE oiio_type = TypeDesc::UINT8;
	static const int opencv_type = CV_8U;
};

template <>
struct ImageTraits<float> {
	static const TypeDesc::BASETYPE oiio_type = TypeDesc::FLOAT;
	static const int opencv_type = CV_32F;
};

template <typename T>
void copyData(ImageInput& input, cv::Mat& m) {
	const ImageSpec& spec = input.spec();
	std::size_t xres = spec.width;
	std::size_t yres = spec.height;
	std::size_t channels = spec.nchannels;

	std::vector<T> pixels(xres * yres * channels);
	input.read_image(ImageTraits<T>::oiio_type, pixels.data());
	input.close();

	m = cv::Mat::zeros(yres, xres, CV_MAKETYPE(ImageTraits<T>::opencv_type, channels));

	tbb::parallel_for(std::size_t(0), yres, [&](std::size_t y) {
		for(std::size_t x = 0; x < xres; ++x) {
			T* ptr = m.ptr<T>(y, x);

			for(std::size_t c = 0; c < channels; ++c) {
				// reverse channels - oiio RGB to opencv BGR
				std::size_t index = channels - c - 1;

				T value = *(pixels.data() + (y * xres + x) * channels + c);
				assert(value > 0 || value == 0);
				*(ptr + index) = value;
			}
		}
	});
}

}  // namespace

std::pair<cv::Mat, Exif> load(const boost::filesystem::path& filename) {
	std::pair<cv::Mat, Exif> result;

	if(!filename.empty() && boost::filesystem::exists(filename)) {
		std::unique_ptr<ImageInput> in(ImageInput::open(filename.string()));
		if(!in)
			throw std::runtime_error("Error loading " + filename.string());

		// get the image spec
		const ImageSpec& spec = in->spec();

		// convert the raw data to a cv::Mat type
		if(spec.format == TypeDesc::UINT8)
			copyData<unsigned char>(*in, result.first);
		else if(spec.format == TypeDesc::FLOAT)
			copyData<float>(*in, result.first);
		else
			throw std::runtime_error("Error loading " + filename.string() +
			                         " - only images with 8 or 32 bits per channel are supported at the moment!");

		// process metadata (EXIF)
		float exposure = 0.0f;
		{
			auto it = spec.extra_attribs.find("ExposureTime");
			if(it != spec.extra_attribs.end()) {
#if OIIO_VERSION_MAJOR < 2
				exposure = (*static_cast<const float*>(it->data()));
#else
				exposure = it->get_float();
#endif
			}
		}

		float fnumber = 0.0f;
		{
			auto it = spec.extra_attribs.find("FNumber");
			if(it != spec.extra_attribs.end()) {
#if OIIO_VERSION_MAJOR < 2
				fnumber = (*static_cast<const float*>(it->data()));
#else
				fnumber = it->get_float();
#endif
			}
		}

		float iso = 100.0f;
		{
			auto it = spec.extra_attribs.find("Exif:PhotographicSensitivity");
			if(it != spec.extra_attribs.end()) {
#if OIIO_VERSION_MAJOR < 2
				iso = (*static_cast<const float*>(it->data()));
#else
				iso = it->get_float();
#endif
			}
		}

		// for(auto& a : spec.extra_attribs)
		// 	std::cout << a.name() << "  " << a.get_float() << std::endl;

		result.second = possumwood::opencv::Exif(exposure, fnumber, iso);
	}
	else
		throw std::runtime_error("Filename '" + filename.string() + "' not found or not accessible!");

	return result;
}

}  // namespace opencv
}  // namespace possumwood
