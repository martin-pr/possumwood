#include <iostream>
#include <string>
#include <stdexcept>

#include <boost/program_options.hpp>

#include <GL/freeglut.h>

#include <QApplication>
#include <QMainWindow>

#include <ImathVec.h>

#include "viewport.h"
#include "bind.h"

namespace po = boost::program_options;

using std::cout;
using std::endl;
using std::flush;

void drawGrid(unsigned x_center, unsigned y_center) {
	glBegin(GL_LINES);
	for(int a=-30;a<=30;++a) {
		glColor3f(0, 0.2 + 0.2*(float)(a % 10 == 0), 0);

		glVertex2f(a*10 + x_center, -300 + y_center);
		glVertex2f(a*10 + x_center, 300 + y_center);

		glVertex2f(-300 + x_center, a*10 + y_center);
		glVertex2f(300 + x_center, a*10 + y_center);
	}
	glEnd();
}

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

	glutInit(&argc, argv);

	QApplication app(argc, argv);

	// make a window only with a viewport
	QMainWindow win;

	win.showMaximized();

	viewport* vp = new viewport(&win);
	win.setCentralWidget(vp);

	///

	qt_bind(vp, SIGNAL(render()), [&]() {
		// draw the grid
		drawGrid(vp->width()/2, vp->height()/2);

		// and request next repaint
		vp->repaint();
	});

	app.exec();

	return 0;
}
