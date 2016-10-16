#include <iostream>
#include <string>
#include <stdexcept>

#include <boost/program_options.hpp>

#include <GL/freeglut.h>

#include <QApplication>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <ImathVec.h>

#include "bind.h"

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

	QGraphicsScene* scene = new QGraphicsScene(&win);

	///

	QGraphicsView* view = new QGraphicsView(scene, &win);
	win.setCentralWidget(view);

	app.exec();

	return 0;
}
