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
std::unique_ptr<RenderContext> ctx;

// global application instance
std::unique_ptr<possumwood::App> papp;

void printHelp() {
	std::cout << "Parameters:" << std::endl;
	std::cout << "  --scene <filename> - Loads a .psw scene file." << std::endl;
	std::cout << "  --render <filename> - renders a frame to a file. Only PPM files supported at the moment." << std::endl;
	std::cout << "  --window <width> <height> - defines the render window size in pixels" << std::endl;
	std::cout << std::endl;
}

void loadScene(const Options::Item& option) {
	if(option.parameters.size() != 1)
		throw std::runtime_error("--scene option allows only exactly one filename");

	std::cout << "Loading " << option.parameters[0] << "... " << std::flush;
	papp->loadFile(boost::filesystem::path(option.parameters[0]));
	std::cout << "done" << std::endl;
}

std::vector<Action> render(const Options::Item& option) {
	if(option.parameters.size() != 1)
		throw std::runtime_error("--render option allows only exactly one filename");

	const std::string filename = option.parameters[0];

	std::function<void(std::vector<GLubyte>&)> callback = [filename](std::vector<GLubyte>& buffer) {
		std::cout << "Rendering " << filename << "... " << std::flush;

		std::ofstream file(filename.c_str(), std::ofstream::binary);

		file << "P6" << std::endl;
		file << viewport.width() << " " << viewport.height() << " 255" << std::endl;

		for(unsigned l=viewport.height(); l>0; --l)
			file.write((const char*)(&buffer[(l-1) * viewport.width()*3]), viewport.width()*3);

		std::cout << "done" << std::endl;
	};

	std::vector<Action> result;
	result.push_back(ctx->render(viewport, callback));
	return result;
}

std::vector<Action> evaluateOption(const Options::const_iterator& current) {
	const Options::Item& option = *current;

	if(option.name == "--scene")
		loadScene(option);

	else if(option.name == "--render")
		return render(option);

	else if(option.name == "--help")
		printHelp();

	else if(option.name == "--window") {
		if(option.parameters.size() != 2)
			throw std::runtime_error("--window option allows only exactly two integer parameters");

		viewport.resize(
			atoi(option.parameters[0].c_str()),
			atoi(option.parameters[1].c_str()));
	}

	else
		throw std::runtime_error("Unknown command line option " + option.name);

	return std::vector<Action>();
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

	// populate the initial action
	Stack s;
	s.add(Action([&]() {
		std::vector<Action> result;
		result.push_back(Action(std::bind(evaluateOptions, std::cref(options))));
		return result;
	}));

	// two types of loops - "non-GL" loop when no --render parameter passed
	//                    - "GL" loop based on GLUT when at least one --render parameter is present
	auto renderParamIt = std::find_if(options.begin(), options.end(),
		[](const Options::Item& item) {
			return item.name == "--render";
		});

	if(renderParamIt != options.end()) {
		// use GL-based loop
		ctx = std::unique_ptr<RenderContext>(new RenderContext(viewport));
		ctx->run(s);
	}

	else {
		// use a trivial while statement
		while(!s.isFinished())
			s.step();
	}

	return 0;
}
