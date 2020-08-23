#include "toolbar.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <streambuf>

#include <boost/filesystem.hpp>

#include <QToolBar>
#include <QToolButton>

#include <actions/actions.h>
#include <possumwood_sdk/app.h>

#include "error_dialog.h"
#include "main_window.h"

namespace {

struct Setup {
	std::string name, description;
};

}  // namespace

Toolbar::Toolbar() {
	const boost::filesystem::path path = possumwood::Filepath::fromString("$TOOLBARS").toPath();

	if(boost::filesystem::exists(path)) {
		std::map<boost::filesystem::path, std::string> paths;
		for(auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(path), {})) {
			static const std::regex regex("^[0-9]+_.*");

			std::string dir = entry.path().filename().string();
			if(std::regex_match(dir, regex))
				dir = dir.substr(dir.find('_') + 1);

			paths[entry.path().filename()] = dir;
		}

		for(auto& toolbarIt : paths) {
			QToolBar* tb = new QToolBar();
			tb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
			tb->setIconSize(QSize(32, 32));

			QFont font = tb->font();
			font.setPointSize(8);
			tb->setFont(font);

			addTab(tb, toolbarIt.second.c_str());

			std::map<boost::filesystem::path, Setup> setups;
			for(auto& entry :
			    boost::make_iterator_range(boost::filesystem::directory_iterator(path / toolbarIt.first), {}))
				if(entry.path().extension() == ".psw") {
					static const std::regex regex("^[0-9]+_.*");

					std::string name = entry.path().filename().stem().string();
					if(std::regex_match(name, regex))
						name = name.substr(name.find('_') + 1);
					std::replace(name.begin(), name.end(), '_', '\n');

					setups[entry.path().filename()].name = name;

					{
						std::ifstream file(entry.path().string());
						possumwood::io::json json;
						file >> json;

						if(!json["description"].is_null())
							setups[entry.path().filename()].description =
							    possumwood::Description(json["description"].get<std::string>()).html();
					}
				}

			unsigned currentIndex = 1;
			for(auto& i : setups) {
				const boost::filesystem::path setupPath = path / toolbarIt.first / i.first;
				boost::filesystem::path iconPath = setupPath;
				iconPath.replace_extension(".png");

				{
					std::stringstream fnss(setupPath.filename().string());
					unsigned index;
					fnss >> index;

					if(index > currentIndex) {
						QFrame* separator = new QFrame();
						separator->setMinimumSize(QSize(10, 1));
						tb->addWidget(separator);

						currentIndex = index;
					}

					++currentIndex;
				}

				QAction* action = tb->addAction(i.second.name.c_str());
				action->setToolTip(i.second.description.c_str());
				if(boost::filesystem::exists(iconPath))
					action->setIcon(QIcon(iconPath.c_str()));

				QToolButton* toolButton = (QToolButton*)tb->widgetForAction(action);
				toolButton->setAutoRaise(false);

				QFont font = toolButton->font();
				font.setPointSize(7);
				toolButton->setFont(font);

				toolButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

				QAction::connect(action, &QAction::triggered, [setupPath]() {
					dependency_graph::State state;

					try {
						QMainWindow* win = possumwood::App::instance().mainWindow();
						MainWindow* mw = dynamic_cast<MainWindow*>(win);
						assert(mw);

						// if adding this to an empty scene, just load the setup and reset the filename
						if(mw->adaptor().currentNetwork().empty())
							mw->loadFile(setupPath.string(), false);

						// otherwise paste the setup into the existing scene
						else {
							std::ifstream file(setupPath.string());
							std::string setup =
							    std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

							dependency_graph::Selection selection;
							state = possumwood::actions::paste(mw->adaptor().currentNetwork(), selection, setup,
							                                   false /* don't halt on error */);
							mw->adaptor().setSelection(selection);
						}
					}
					catch(std::exception& e) {
						state.addError(std::string("Error inserting setup - ") + e.what());
					}
					catch(...) {
						state.addError("Error inserting setup - unknown exception.");
					}

					// if any non-critical error happened, let's just print it out for now
					if(state.errored()) {
						ErrorDialog* err = new ErrorDialog(state, possumwood::App::instance().mainWindow());
						err->show();
					}
				});
			}
		}
	}
}
