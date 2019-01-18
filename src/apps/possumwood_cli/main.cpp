#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GL/glut.h>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/viewport_state.h>

#include "common.h"
#include "options.h"
#include "render_context.h"
#include "stack.h"


// The CLI unfortunately has to be designed around GLUT - it is not possible in current
// version of GLUT to re-enter the event loop, and OSmesa does not support GL>2.0 on
// Debian.

// The RenderContext object provides an "execution engine" wired around GLUT to evaluate
// each step of execution inside the OpenGL loop. It initialises the OpenGL on the first
// --render parameter, and destroys it once the evaluation of all parameters finishes.


// global viewport state
possumwood::ViewportState viewport;
// rendering options
RenderContext ctx(viewport);

// global application instance
std::unique_ptr<possumwood::App> papp;

std::vector<Action> evaluateOption(const Options::const_iterator& current) {
	const Options::Item& option = *current;

	if(option.name == "--scene") {
		if(option.parameters.size() != 1)
			throw std::runtime_error("--scene option allows only exactly one filename");

		std::cout << "Loading " << option.parameters[0] << "... " << std::flush;
		papp->loadFile(boost::filesystem::path(option.parameters[0]));
		std::cout << "done" << std::endl;
	}

	// rendering a frame
	else if(option.name == "--render") {
		if(option.parameters.size() != 1)
			throw std::runtime_error("--render option allows only exactly one filename");

		std::cout << "Rendering " << option.parameters[0] << "... " << std::flush;

		std::vector<GLubyte> buffer = ctx.render(viewport);

		{
			std::ofstream file(option.parameters[0].c_str(), std::ofstream::binary);

			file << "P6" << std::endl;
			file << viewport.width << " " << viewport.height << " 255" << std::endl;

			for(unsigned l=viewport.height; l>0; --l)
				file.write((const char*)(&buffer[(l-1) * viewport.width*3]), viewport.width*3);
		}


		std::cout << "done" << std::endl;
	}

	else
		throw std::runtime_error("Unknown command line option " + option.name);

	std::vector<Action> result;
	return result;
}

std::vector<Action> evaluateOptions(const Options& options) {
	std::vector<Action> result;

	for(auto it = options.begin(); it != options.end(); ++it)
		result.push_back(Action(std::bind(evaluateOption, it)));

	return result;
}

int main(int argc, char* argv[]) {
	// create the possumwood application
	papp = std::unique_ptr<possumwood::App>(new possumwood::App());

	// load all plugins into an RAII container
	PluginsRAII plugins;

	// parse the program options
	Options options(argc, argv);

	// and run the loop
	Stack s;
	s.add(Action([&]() {
		std::vector<Action> result;
		result.push_back(Action(std::bind(evaluateOptions, std::cref(options))));
		return result;
	}));

	while(!s.isFinished())
		s.step();

	return 0;
}
