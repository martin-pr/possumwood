#include "raw.h"

#include <cassert>
#include <sstream>

#include <nlohmann/json.hpp>

#include "block.h"
#include "metadata.h"

namespace lightfields {

namespace {

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

}  // namespace

struct Raw::Pimpl {
	Metadata meta;
	std::unique_ptr<unsigned char[]> data;
};

Raw::Raw() : m_pimpl(new Pimpl()) {
}

Raw::~Raw() {
}

const Metadata& Raw::metadata() const {
	return m_pimpl->meta;
}

nlohmann::json& Raw::header(Pimpl& p) {
	return p.meta.m_header;
}
nlohmann::json& Raw::meta(Pimpl& p) {
	return p.meta.m_meta;
}
nlohmann::json& Raw::privateMeta(Pimpl& p) {
	return p.meta.m_privateMeta;
}

const unsigned char* Raw::image() const {
	return m_pimpl->data.get();
}

std::istream& operator>>(std::istream& in, Raw& data) {
	std::unique_ptr<Raw::Pimpl> impl(new Raw::Pimpl());

	std::string metadataRef, privateMetadataRef, imageRef;

	lightfields::Block block;

	// skip the initial block
	in >> block;
	checkThrow(block.id, 'P', "Initial P block of a lytro raw file not found.");

	while(block.id != '\0') {
		in >> block;

		if(block.id == 'M') {
			std::stringstream ss((const char*)block.data.get());
			ss >> Raw::header(*impl);

			if(impl->meta.header()["frames"].size() != 1)
				throw std::runtime_error("Only single-frame raw images supported at the moment.");

			metadataRef = impl->meta.header()["frames"][0]["frame"]["metadataRef"].get<std::string>();
			privateMetadataRef = impl->meta.header()["frames"][0]["frame"]["privateMetadataRef"].get<std::string>();
			imageRef = impl->meta.header()["frames"][0]["frame"]["imageRef"].get<std::string>();
		}

		else if(block.name == metadataRef) {
			std::stringstream ss((const char*)block.data.get());
			ss >> Raw::meta(*impl);

			checkThrow(impl->meta.metadata()["image"]["width"].is_number_integer(), true, "width");
			checkThrow(impl->meta.metadata()["image"]["height"].is_number_integer(), true, "height");

			checkThrow(impl->meta.metadata()["image"]["orientation"].get<int>(), 1, "orientation");
			checkThrow(impl->meta.metadata()["image"]["representation"].get<std::string>(), std::string("rawPacked"),
			           "representation");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["rightShift"].get<int>(), 0,
			           "rightShift");

			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["black"].size(), std::size_t(4),
			           "black size");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["white"].size(), std::size_t(4),
			           "white size");

			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["black"]["b"].is_number_integer(),
			           true, "[black][b]");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["black"]["gb"].is_number_integer(),
			           true, "[black][gb]");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["black"]["gr"].is_number_integer(),
			           true, "[black][gr]");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["black"]["r"].is_number_integer(),
			           true, "[black][r]");

			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["white"]["b"].is_number_integer(),
			           true, "[white][b]");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["white"]["gb"].is_number_integer(),
			           true, "[white][gb]");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["white"]["gr"].is_number_integer(),
			           true, "[white][gr]");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelFormat"]["white"]["r"].is_number_integer(),
			           true, "[white][r]");

			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelPacking"]["endianness"].get<std::string>(),
			           std::string("big"), "endianness");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["pixelPacking"]["bitsPerPixel"].get<int>(), 12,
			           "bitsPerPixel");

			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["mosaic"]["tile"].get<std::string>(),
			           std::string("r,gr:gb,b"), "mosaic/tile");
			checkThrow(impl->meta.metadata()["image"]["rawDetails"]["mosaic"]["upperLeftPixel"].get<std::string>(),
			           std::string("b"), "mosaic/upperLeftPixel");
		}

		else if(block.name == privateMetadataRef) {
			std::stringstream ss((const char*)block.data.get());
			ss >> Raw::privateMeta(*impl);
		}

		// just copy the image data
		else if(block.name == imageRef) {
			assert(block.data != nullptr);
			impl->data = std::move(block.data);
		}

		// temporary - printouts of headers
		// if((block.data.size() > 1) && (block.data[0] == '{'))
		// 	std::cout << block.data.data() << std::endl;
	}

	data.m_pimpl = std::shared_ptr<const Raw::Pimpl>(impl.release());

	return in;
}

}  // namespace lightfields
