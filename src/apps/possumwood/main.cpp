#include <dependency_graph/graph.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>
#include <qt_node_editor/connected_edge.h>
#include <qt_node_editor/graph_widget.h>
#include <qt_node_editor/node.h>

#include <Eigen/Core>
#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/node_base.inl>
#include <iostream>
#include <stdexcept>
#include <string>

#include "adaptor.h"
#include "common.h"
#include "error_dialog.h"
#include "gl_init.h"
#include "main_window.h"

namespace po = boost::program_options;

using std::cout;
using std::endl;
using std::flush;

int main(int argc, char* argv[]) {
	// // Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()("help", "produce help message")("scene", po::value<std::string>(), "open a scene file");

	// process the options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}

	///////////////////////////////

	// create the possumwood application
	std::unique_ptr<possumwood::App> papp(new possumwood::App());
	// load all plugins into an RAII container - loading of plugins requires path expansion from the main app instance
	std::unique_ptr<PluginsRAII> plugins(new PluginsRAII());

	{
		GL_CHECK_ERR;

		{
			QSurfaceFormat format = QSurfaceFormat::defaultFormat();
			// way higher than currently supported - will fall back to highest
			format.setVersion(6, 0);
			format.setProfile(QSurfaceFormat::CoreProfile);

#ifndef NDEBUG
			format.setOption(QSurfaceFormat::DebugContext);
#endif

			format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

			QSurfaceFormat::setDefaultFormat(format);
		}

		// initialise eigen
		Eigen::initParallel();

		GL_CHECK_ERR;

// nice scaling on High DPI displays is support in Qt 5.6+
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
		QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

		GL_CHECK_ERR;

		// create the application object
		QApplication app(argc, argv);

		GL_CHECK_ERR;

		// make a main window
		MainWindow win;
		win.setWindowIcon(QIcon(":icons/app.png"));
		win.showMaximized();

		GL_CHECK_ERR;

		std::cout << "OpenGL version supported by this platform is " << glGetString(GL_VERSION) << std::endl;

		GL_CHECK_ERR;

		// open the scene file, if specified on the command line
		if(vm.count("scene"))
			win.loadFile(vm["scene"].as<std::string>());

		GL_CHECK_ERR;

		// and start the main application loop
		app.exec();

		GL_CHECK_ERR;
	}

	// delete the application before unloading plugins - app will need metadata from plugins to close cleanly
	papp.reset();
	plugins.reset();

	return 0;
}
