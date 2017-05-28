#include <iostream>
#include <string>
#include <stdexcept>

#include <boost/program_options.hpp>

#include <GL/freeglut.h>

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QHBoxLayout>

#include <ImathVec.h>

#include "bind.h"
#include "node.h"
#include "connected_edge.h"
#include "graph_widget.h"
#include <dependency_graph/graph.h>
#include <dependency_graph/node.inl>
#include <dependency_graph/datablock.inl>
#include "node_data.h"
#include <dependency_graph/metadata.inl>
#include <dependency_graph/attr.inl>
#include "adaptor.h"
#include "main_window.h"

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

	// create the application object
	QApplication app(argc, argv);

	// make a main window
	MainWindow win;
	win.showMaximized();

	// and start the main application loop
	app.exec();

	return 0;
}
