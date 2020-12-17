#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>

#include <tbb/parallel_for.h>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>
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
dependency_graph::InAttr<possumwood::Enum> a_debayer;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::OutAttr<lightfields::Metadata> a_metadata;

static std::vector<std::pair<std::string, int>> s_debayer{
    {"None", lightfields::Bayer::kNone},
    {"Basic", lightfields::Bayer::kBasic},
    {"Edge-aware", lightfields::Bayer::kEA},
};

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
		std::ifstream file(filename.filename().string(), std::ios::binary);
		lightfields::Raw raw;
		file >> raw;

		lightfields::Bayer bayer(raw.metadata().metadata());

		assert(raw.image() != nullptr);
		result = bayer.decode(raw.image(), (lightfields::Bayer::Decoding)data.get(a_debayer).intValue());

		meta = raw.metadata();
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
	meta.addAttribute(a_debayer, "debayer", possumwood::Enum(s_debayer.begin(), s_debayer.end()));
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_metadata, "metadata", lightfields::Metadata(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_filename, a_frame);
	meta.addInfluence(a_filename, a_metadata);
	meta.addInfluence(a_debayer, a_frame);
	meta.addInfluence(a_debayer, a_metadata);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/lytro_lightfield", init);

}  // namespace
