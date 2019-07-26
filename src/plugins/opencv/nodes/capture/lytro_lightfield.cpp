#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <sstream>
#include <fstream>

#include <boost/filesystem.hpp>

#include <actions/traits.h>
#include <actions/io/json.h>

#include <opencv2/opencv.hpp>

#include "frame.h"
#include "lightfield_pattern.h"
#include "lightfields/block.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::OutAttr<possumwood::opencv::LightfieldPattern> a_pattern;

cv::Mat decodeData(const char* data, std::size_t width, std::size_t height, int black[4], int white[4]) {
	cv::Mat result(width, height, CV_32F);

	for(std::size_t i=0; i<width*height; ++i) {
		const unsigned char c1 = data[i/2*3];
		const unsigned char c2 = data[i/2*3+1];
		const unsigned char c3 = data[i/2*3+2];

		unsigned short val;
		if(i%2 == 0)
			val = ((unsigned short)c1 << 4) + (unsigned short)c2;
		else
			val = (((unsigned short)c2 & 0x0f) << 8) + (unsigned short)c3;

		const unsigned patternId = (i%width) % 2 + ((i/width)%2)*2;

		const float fval = ((float)val - (float)black[patternId]) / ((float)(white[patternId]-black[patternId]));

		result.at<float>(i/width, i%width) = fval;
	}

	return result;
}

template<typename T>
std::string str(const T& val) {
	return std::to_string(val);
}

template<>
std::string str<std::string>(const std::string& val) {
	return val;
}

template<typename T>
void checkThrow(const T& value, const T& expected, const std::string& error) {
	if(value != expected)
		throw std::runtime_error("Expected " + error + " " + str(expected) + ", got " + str(value) + "!");
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	cv::Mat result;
	possumwood::opencv::LightfieldPattern pattern;

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename())) {
		int width = 0, height = 0;
		std::string metadataRef, imageRef;
		int black[4] = {0,0,0,0}, white[4] = {255,255,255,255};

		std::ifstream file(filename.filename().string(), std::ios::binary);
		lightfields::Block block;

		// skip the initial block
		file >> block;
		checkThrow(block.id, 'P', "Initial P block of a lytro raw file not found.");

		while(block.id != '\0') {
			file >> block;

			if(block.id == 'M') {
				std::stringstream ss(block.data.data());
				possumwood::io::json metadata;
				ss >> metadata;

				if(metadata["frames"].size() != 1)
					throw std::runtime_error("Only single-frame raw images supported at the moment.");

				metadataRef = metadata["frames"][0]["frame"]["metadataRef"];
				imageRef = metadata["frames"][0]["frame"]["imageRef"];
			}

			else if(block.name == metadataRef) {
				std::stringstream ss(block.data.data());
				possumwood::io::json metadata;
				ss >> metadata;

				width = metadata["image"]["width"].get<int>();
				height = metadata["image"]["height"].get<int>();

				checkThrow(metadata["image"]["orientation"].get<int>(), 1, "orientation");
				checkThrow(metadata["image"]["representation"].get<std::string>(), std::string("rawPacked"), "representation");
				checkThrow(metadata["image"]["rawDetails"]["pixelFormat"]["rightShift"].get<int>(), 0, "rightShift");

				checkThrow(metadata["image"]["rawDetails"]["pixelFormat"]["black"].size(), std::size_t(4), "black size");
				checkThrow(metadata["image"]["rawDetails"]["pixelFormat"]["white"].size(), std::size_t(4), "white size");

				black[0] = metadata["image"]["rawDetails"]["pixelFormat"]["black"]["b"].get<int>();
				black[1] = metadata["image"]["rawDetails"]["pixelFormat"]["black"]["gb"].get<int>();
				black[2] = metadata["image"]["rawDetails"]["pixelFormat"]["black"]["gr"].get<int>();
				black[3] = metadata["image"]["rawDetails"]["pixelFormat"]["black"]["r"].get<int>();

				white[0] = metadata["image"]["rawDetails"]["pixelFormat"]["white"]["b"].get<int>();
				white[1] = metadata["image"]["rawDetails"]["pixelFormat"]["white"]["gb"].get<int>();
				white[2] = metadata["image"]["rawDetails"]["pixelFormat"]["white"]["gr"].get<int>();
				white[3] = metadata["image"]["rawDetails"]["pixelFormat"]["white"]["r"].get<int>();

				checkThrow(metadata["image"]["rawDetails"]["pixelPacking"]["endianness"].get<std::string>(), std::string("big"), "endianness");
				checkThrow(metadata["image"]["rawDetails"]["pixelPacking"]["bitsPerPixel"].get<int>(), 12, "bitsPerPixel");

				checkThrow(metadata["image"]["rawDetails"]["mosaic"]["tile"].get<std::string>(), std::string("r,gr:gb,b"), "mosaic/tile");
				checkThrow(metadata["image"]["rawDetails"]["mosaic"]["upperLeftPixel"].get<std::string>(), std::string("b"), "mosaic/upperLeftPixel");

				// assemble the lightfield pattern
				pattern = possumwood::opencv::LightfieldPattern(
					metadata["devices"]["mla"]["lensPitch"].get<double>(),
					metadata["devices"]["sensor"]["pixelPitch"].get<double>(),
					metadata["devices"]["mla"]["rotation"].get<double>(),
					cv::Vec2f(
						metadata["devices"]["mla"]["scaleFactor"]["x"].get<double>(),
						metadata["devices"]["mla"]["scaleFactor"]["y"].get<double>()
					),
					cv::Vec3f(
						metadata["devices"]["mla"]["sensorOffset"]["x"].get<double>(),
						metadata["devices"]["mla"]["sensorOffset"]["y"].get<double>(),
						metadata["devices"]["mla"]["sensorOffset"]["z"].get<double>()
					),
					cv::Vec2i(width, height)
				);
			}
			else if(block.name == imageRef) {
				result = decodeData(block.data.data(), width, height, black, white);
			}

			// temporary - printouts
			if((block.data.size() > 1) && (block.data[0] == '{'))
				std::cout << block.data.data() << std::endl;
		}
	}

	data.set(a_frame, possumwood::opencv::Frame(result));
	data.set(a_pattern, pattern);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"Lytro files (*.lfr *.RAW)",
	}));
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_pattern, "pattern", possumwood::opencv::LightfieldPattern(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_filename, a_frame);
	meta.addInfluence(a_filename, a_pattern);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/lytro_lightfield", init);

}
