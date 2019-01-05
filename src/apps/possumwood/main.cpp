#include <iostream>
#include <string>
#include <stdexcept>

#include <dlfcn.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QHBoxLayout>

#include <dependency_graph/graph.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/attr.inl>

#include <qt_node_editor/node.h>
#include <qt_node_editor/connected_edge.h>
#include <qt_node_editor/graph_widget.h>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl.h>

#include "adaptor.h"
#include "main_window.h"
#include "gl_init.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

using std::cout;
using std::endl;
using std::flush;

int main(int argc, char* argv[]) {
	// // Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("scene", po::value<std::string>(), "open a scene file")
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

	// create the possumwood application
	possumwood::App papp;

	std::vector<void*> pluginHandles;

	// scan for plugins
	for(fs::directory_iterator itr(papp.expandPath("$PLUGINS"));
	    itr != fs::directory_iterator(); ++itr) {
		if(fs::is_regular_file(itr->status()) && itr->path().extension() == ".so") {
			void* ptr = dlopen(itr->path().string().c_str(), RTLD_NOW);
			if(ptr)
				pluginHandles.push_back(ptr);
			else
				std::cout << dlerror() << std::endl;
		}
	}

	///////////////////////////////

	{
		GL_CHECK_ERR;

		{
			QSurfaceFormat format = QSurfaceFormat::defaultFormat();
			// way higher than currently supported - will fall back to highest
			format.setVersion(6, 0);
			format.setProfile(QSurfaceFormat::CoreProfile);

			format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

			QSurfaceFormat::setDefaultFormat(format);
		}

		// create the application object
		QApplication app(argc, argv);

		// nice scaling on High DPI displays is support in Qt 5.6+
		#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
			QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
			QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
		#endif

		GL_CHECK_ERR;

		// make a main window
		MainWindow win;
		win.setWindowIcon(QIcon(":icons/app.png"));
		win.showMaximized();

		GL_CHECK_ERR;

		std::cout << "OpenGL version supported by this platform is "
		          << glGetString(GL_VERSION) << std::endl;

		GL_CHECK_ERR;

		// open the scene file, if specified on the command line
		if(vm.count("scene"))
			possumwood::App::instance().loadFile(boost::filesystem::path(vm["scene"].as<std::string>()));

		GL_CHECK_ERR;

		// and start the main application loop
		app.exec();

		GL_CHECK_ERR;
	}

	////////////////////////////////

	// unload all plugins
	while(!pluginHandles.empty()) {
		dlclose(pluginHandles.back());
		pluginHandles.pop_back();
	}

	return 0;
}
