#include <iostream>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/viewport_state.h>

#include "common.h"
#include "options.h"
#include "render_context.h"

int main(int argc, char* argv[]) {
	// create the possumwood application
	possumwood::App papp;

	// load all plugins into an RAII container
	PluginsRAII plugins;

	// parse the program options
	Options options(argc, argv);

	// rendering options
	possumwood::ViewportState viewport;

	for(auto& option : options) {
		// scene loading
		if(option.name == "--scene") {
			if(option.parameters.size() != 1)
				throw std::runtime_error("--scene option allows only exactly one filename");

			std::cout << "Loading " << option.parameters[0] << "... " << std::flush;
			papp.loadFile(boost::filesystem::path(option.parameters[0]));
			std::cout << "done" << std::endl;
		}

		// rendering a frame
		else if(option.name == "--render") {
			if(option.parameters.size() != 1)
				throw std::runtime_error("--render option allows only exactly one filename");

			std::cout << "Rendering " << option.parameters[0] << "... " << std::flush;

			RenderContext ctx;
			std::vector<GLubyte> buffer = ctx.render(viewport);

			std::cout << "done" << std::endl;
		}

		else
			throw std::runtime_error("Unknown command line option " + option.name);
	}

	return 0;
}
