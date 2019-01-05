#include <iostream>

#include <possumwood_sdk/app.h>

#include "common.h"
#include "options.h"

int main(int argc, char* argv[]) {
	// create the possumwood application
	possumwood::App papp;

	// load all plugins into an RAII container
	PluginsRAII plugins;

	// parse the program options
	Options options(argc, argv);

	for(auto& option : options) {
		// scene loading
		if(option.name == "--scene") {
			if(option.parameters.size() != 1)
				throw std::runtime_error("--scene option allows only exactly one filename");

			std::cout << "Loading " << option.parameters[0] << "... " << std::flush;
			papp.loadFile(boost::filesystem::path(option.parameters[0]));
			std::cout << "done" << std::endl;
		}
		else
			throw std::runtime_error("Unknown command line option " + option.name);
	}

	return 0;
}
