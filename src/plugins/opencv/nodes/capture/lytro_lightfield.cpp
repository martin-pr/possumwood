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
#include "lightfields/raw.h"
#include "lightfields/pattern.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::OutAttr<lightfields::Pattern> a_pattern;

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
	lightfields::Pattern pattern;

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename())) {
		int width = 0, height = 0;
		std::string metadataRef, imageRef;
		int black[4] = {0,0,0,0}, white[4] = {255,255,255,255};

		std::ifstream file(filename.filename().string(), std::ios::binary);
		lightfields::Raw raw;
		file >> raw;

		width = raw.metadata()["image"]["width"].asInt();
		height = raw.metadata()["image"]["height"].asInt();

		black[0] = raw.metadata()["image"]["rawDetails"]["pixelFormat"]["black"]["b"].asInt();
		black[1] = raw.metadata()["image"]["rawDetails"]["pixelFormat"]["black"]["gb"].asInt();
		black[2] = raw.metadata()["image"]["rawDetails"]["pixelFormat"]["black"]["gr"].asInt();
		black[3] = raw.metadata()["image"]["rawDetails"]["pixelFormat"]["black"]["r"].asInt();

		white[0] = raw.metadata()["image"]["rawDetails"]["pixelFormat"]["white"]["b"].asInt();
		white[1] = raw.metadata()["image"]["rawDetails"]["pixelFormat"]["white"]["gb"].asInt();
		white[2] = raw.metadata()["image"]["rawDetails"]["pixelFormat"]["white"]["gr"].asInt();
		white[3] = raw.metadata()["image"]["rawDetails"]["pixelFormat"]["white"]["r"].asInt();

		// assemble the lightfield pattern
		pattern = lightfields::Pattern(
			raw.metadata()["devices"]["mla"]["lensPitch"].asDouble(),
			raw.metadata()["devices"]["sensor"]["pixelPitch"].asDouble(),
			raw.metadata()["devices"]["mla"]["rotation"].asDouble(),
			Imath::V2f(
				raw.metadata()["devices"]["mla"]["scaleFactor"]["x"].asDouble(),
				raw.metadata()["devices"]["mla"]["scaleFactor"]["y"].asDouble()
			),
			Imath::V3f(
				raw.metadata()["devices"]["mla"]["sensorOffset"]["x"].asDouble(),
				raw.metadata()["devices"]["mla"]["sensorOffset"]["y"].asDouble(),
				raw.metadata()["devices"]["mla"]["sensorOffset"]["z"].asDouble()
			),
			Imath::V2i(width, height)
		);

		assert(!raw.image().empty());
		result = decodeData(raw.image().data(), width, height, black, white);
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
	meta.addAttribute(a_pattern, "pattern", lightfields::Pattern(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_filename, a_frame);
	meta.addInfluence(a_filename, a_pattern);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/lytro_lightfield", init);

}
