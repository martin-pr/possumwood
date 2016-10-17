#include <iostream>
#include <string>
#include <stdexcept>

#include <boost/program_options.hpp>

#include <GL/freeglut.h>

#include <QApplication>
#include <QMainWindow>

#include <ImathVec.h>

#include "bind.h"
#include "node.h"
#include "graph_widget.h"

namespace po = boost::program_options;

using std::cout;
using std::endl;
using std::flush;

int main(int argc, char* argv[]) {
	// // Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	("help", "produce help message")
	;

	// process the options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}

	///////////////////////////////

	QApplication app(argc, argv);

	// make a window only with a viewport
	QMainWindow win;

	win.showMaximized();

	///

	GraphWidget* graph = new GraphWidget(&win);
	win.setCentralWidget(graph);

	graph->addNode("first", QPointF(-20, -20), {{"aaaaa", Port::kInput}, {"b", Port::kOutput}});
	graph->addNode("second", QPointF(20, -20), {{"xxxxxxxxxxxxxxxx", Port::kInputOutput}});

	///

	app.exec();

	return 0;
}
