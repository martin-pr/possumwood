#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>

#include <tbb/parallel_for.h>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include "frame.h"
#include "lightfields.h"
#include "lightfields/bayer.h"
#include "lightfields/block.h"
#include "lightfields/metadata.h"
#include "lightfields/pattern.h"
#include "lightfields/raw.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::OutAttr<lightfields::Metadata> a_metadata;

cv::Mat decodeData(const unsigned char* data,
                   std::size_t width,
                   std::size_t height,
                   const lightfields::Bayer::Value& black,
                   const lightfields::Bayer::Value& white) {
	cv::Mat result(width, height, CV_32F);

	tbb::parallel_for(std::size_t(0), width * height, [&](std::size_t i) {
		const uint16_t c1 = data[i / 2 * 3];
		const uint16_t c2 = data[i / 2 * 3 + 1];
		const uint16_t c3 = data[i / 2 * 3 + 2];

		uint16_t val;
		if(i % 2 == 0)
			val = (c1 << 4) + (c2 >> 4);
		else
			val = ((c2 & 0x0f) << 8) + c3;

		const unsigned patternId = (i % width) % 2 + ((i / width) % 2) * 2;

		const float fval = ((float)val - (float)black[patternId]) / ((float)(white[patternId] - black[patternId]));

		result.at<float>(i / width, i % width) = fval;
	});

	return result;
}

template <typename T>
std::string str(const T& val) {
	return std::to_string(val);
}

template <>
std::string str<std::string>(const std::string& val) {
	return val;
}

template <typename T>
void checkThrow(const T& value, const T& expected, const std::string& error) {
	if(value != expected)
		throw std::runtime_error("Expected " + error + " " + str(expected) + ", got " + str(value) + "!");
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	cv::Mat result;
	lightfields::Pattern pattern;
	lightfields::Metadata meta;

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename())) {
		int width = 0, height = 0;
		std::string metadataRef, imageRef;
		lightfields::Bayer::Value black, white;

		std::ifstream file(filename.filename().string(), std::ios::binary);
		lightfields::Raw raw;
		file >> raw;

		meta = raw.metadata();

		width = meta.metadata()["image"]["width"].get<int>();
		height = meta.metadata()["image"]["height"].get<int>();

		black = lightfields::Bayer::Value(meta.metadata()["image"]["rawDetails"]["pixelFormat"]["black"]);
		white = lightfields::Bayer::Value(meta.metadata()["image"]["rawDetails"]["pixelFormat"]["white"]);

		assert(raw.image() != nullptr);
		result = decodeData(raw.image(), width, height, black, white);
	}

	data.set(a_frame, possumwood::opencv::Frame(result));
	data.set(a_metadata, meta);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename",
	                  possumwood::Filename({
	                      "Lytro files (*.lfr *.RAW)",
	                  }));
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_metadata, "metadata", lightfields::Metadata(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_filename, a_frame);
	meta.addInfluence(a_filename, a_metadata);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/lytro_lightfield", init);

}  // namespace
