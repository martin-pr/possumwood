#include "datatypes/font.h"

#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <boost/filesystem.hpp>

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;

dependency_graph::OutAttr<possumwood::Font> a_font;

dependency_graph::State compute(dependency_graph::Values& data) {
	const boost::filesystem::path filename = data.get(a_filename).filename();

	if(!boost::filesystem::exists(filename) || filename.empty())
		throw std::runtime_error("Filename " + filename.string() + " not found.");

	possumwood::Font font;
	font.load(filename);

	data.set(a_font, font);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "font_def_file");
	meta.addAttribute(a_font, "font");

	meta.addInfluence(a_filename, a_font);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("render/font", init);

}  // namespace
