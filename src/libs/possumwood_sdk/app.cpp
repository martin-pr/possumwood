#include "app.h"

#include <cassert>
#include <fstream>
#include <regex>

#include <boost/filesystem.hpp>

#include <QApplication>
#include <QMainWindow>

#include <dependency_graph/port.inl>
#include <dependency_graph/node.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes_iterator.inl>

#include <actions/actions.h>

#include <possumwood_sdk/gl.h>
#include <possumwood_sdk/metadata.h>

#include "config.inl"

namespace possumwood {

App* App::s_instance = NULL;

App::App() : m_mainWindow(NULL), m_time(0.0f) {
	assert(s_instance == nullptr);
	s_instance = this;

	////////////////////////
	// scene configuration

	m_sceneConfig.addItem(Config::Item("start_time", "timeline", 0.0f,
	                                   Config::Item::kNoFlags,
	                                   "Start of the timeline (seconds)"));

	m_sceneConfig.addItem(Config::Item("end_time", "timeline", 5.0f,
	                                   Config::Item::kNoFlags,
	                                   "End of the timeline (seconds)"));

	m_sceneConfig.addItem(Config::Item("fps", "timeline", 24.0f, Config::Item::kNoFlags,
	                                   "Scene's frames-per-second value"));

	///////////////////////
	// expressions

	// find the config file
	boost::filesystem::path full_path(boost::filesystem::current_path());
	while(!full_path.empty()) {
		if(boost::filesystem::exists(full_path / "possumwood.conf"))
			break;

		full_path = full_path.parent_path();
	}

	if(!full_path.empty()) {
		std::ifstream cfg(((full_path / "possumwood.conf").string()));
		while(!cfg.eof() && cfg.good()) {
			std::string line;
			std::getline(cfg, line);

			if(!line.empty()) {
				std::string key;
				std::string value;

				int state = 0;
				for(std::size_t c=0;c<line.length(); ++c) {
					if(state == 0) {
						if(line[c] != ' ' && line[c] != '\t')
							key.push_back(line[c]);
						else
							state = 1;
					}

					if(state == 1) {
						if(line[c] != ' ' && line[c] != '\t')
							state = 2;
					}

					if(state == 2)
						value.push_back(line[c]);
				}

				boost::filesystem::path path(value);
				if(path.is_relative())
					path = full_path / path;

				m_pathVariables[key] = path.string();
			}
		}
	}
	else
		std::cout << "Warning: configuration file 'possumwood.conf' not found!" << std::endl;
}

App::~App() {
	assert(s_instance == this);
	s_instance = nullptr;
}

App& App::instance() {
	assert(s_instance != nullptr);
	return *s_instance;
}

const boost::filesystem::path& App::filename() const {
	return m_filename;
}

void App::newFile() {
	graph().clear();
	m_filename = "";
}

void App::loadFile(const possumwood::io::json& json) {
	// read the graph
	graph().clear();
	undoStack().clear();

	dependency_graph::Selection selection; // throwaway
	possumwood::actions::fromJson(graph(), selection, json);

	undoStack().clear();

	// and read the scene config
	if(json.find("scene_config") != json.end()) {
		auto& config = json["scene_config"];

		for(auto it = config.begin(); it != config.end(); ++it) {
			auto& item = possumwood::App::instance().sceneConfig()[it.key()];

			if(item.is<int>())
				item = it.value().get<int>();
			else if(item.is<float>())
				item = it.value().get<float>();
			else if(item.is<std::string>())
				item = it.value().get<std::string>();
			else
				assert(false);
		}
	}

	// read the UI configuration
	if(QCoreApplication::instance() != nullptr) {
		if(json.find("ui_geometry") != json.end())
			mainWindow()->restoreGeometry(
			    QByteArray::fromBase64(json["ui_geometry"].get<std::string>().c_str()));
		if(json.find("ui_state") != json.end())
			mainWindow()->restoreState(
			    QByteArray::fromBase64(json["ui_state"].get<std::string>().c_str()));
	}
}

void App::loadFile(const boost::filesystem::path& filename) {
	if(!boost::filesystem::exists(filename))
		throw std::runtime_error("Cannot open " + filename.string() +
		                         " - file not found.");
	// read the json file
	std::ifstream in(filename.string());
	possumwood::io::json json;
	in >> json;

	// update the opened filename
	const boost::filesystem::path oldFilename = m_filename;
	m_filename = filename;

	try {
		loadFile(json);
	}
	catch(...) {
		// something bad happened - return back the filename value
		m_filename = oldFilename;

		throw;
	}
}

void App::saveFile() {
	assert(!m_filename.empty());
	saveFile(m_filename);
}

void App::saveFile(possumwood::io::json& json, bool saveSceneConfig) {
	// make a json instance containing the graph
	json = possumwood::actions::toJson();

	// save scene config into the json object
	if(saveSceneConfig) {
		auto& config = json["scene_config"];
		for(auto& i : possumwood::App::instance().sceneConfig())
			if(i.is<int>())
				config[i.name()] = i.as<int>();
			else if(i.is<float>())
				config[i.name()] = i.as<float>();
			else if(i.is<std::string>())
				config[i.name()] = i.as<std::string>();
			else
				assert(false);
	}

	// and save the UI configuration
	if(QCoreApplication::instance() != nullptr) {
		json["ui_geometry"] = mainWindow()->saveGeometry().toBase64().data();
		json["ui_state"] = mainWindow()->saveState().toBase64().data();
	}
}

void App::saveFile(const boost::filesystem::path& fn) {
	possumwood::io::json json;
	saveFile(json);

	// save the json to the file
	std::ofstream out(fn.string());
	out << std::setw(4) << json;

	// and update the filename
	m_filename = fn;
}

QMainWindow* App::mainWindow() const {
	return m_mainWindow;
}

void App::setMainWindow(QMainWindow* win) {
	assert(m_mainWindow == NULL &&
	       "setMainWindow is called only once at the beginning of an application");
	m_mainWindow = win;
}

void App::draw(const possumwood::ViewportState& viewport, std::function<void(const dependency_graph::NodeBase&)> stateChangedCallback) {
	GL_CHECK_ERR;

	for(auto it = graph().nodes().begin(dependency_graph::Nodes::kRecursive); it != graph().nodes().end(); ++it) {
		GL_CHECK_ERR;

		boost::optional<possumwood::Drawable&> drawable = possumwood::Metadata::getDrawable(*it);
		if(drawable) {
			const auto currentDrawState = drawable->drawState();

			GL_CHECK_ERR;
			drawable->doDraw(viewport);
			GL_CHECK_ERR;

			if(drawable->drawState() != currentDrawState && stateChangedCallback)
				stateChangedCallback(*it);
		}

		GL_CHECK_ERR;
	}

	GL_CHECK_ERR;
}

void App::setTime(float time) {
	if(m_time != time) {
		m_time = time;
		m_timeChanged(time);

		// TERRIBLE HACK - a special node type that outputs time is handled here
		for(dependency_graph::Nodes::iterator i = graph().nodes().begin(dependency_graph::Nodes::kRecursive); i != graph().nodes().end(); ++i)
			if(i->metadata()->type() == "time")
				i->port(0).set<float>(time);
	}
}

float App::time() const {
	return m_time;
}

boost::signals2::connection App::onTimeChanged(std::function<void(float)> fn) {
	return m_timeChanged.connect(fn);
}

Config& App::sceneConfig() {
	return m_sceneConfig;
}

boost::filesystem::path App::expandPath(const boost::filesystem::path& path) const {
	std::string p = path.string();

	const std::regex var("\\$[A-Z]+");
	std::smatch match;
	while(std::regex_search(p, match, var)) {
		auto it = m_pathVariables.find(match.str().substr(1));
		if(it != m_pathVariables.end())
			p = match.prefix().str() + it->second.string() + match.suffix().str();
		else
			break;
	}

	return p;
}

boost::filesystem::path App::shrinkPath(const boost::filesystem::path& path) const {
	std::string p = path.string();

	bool cont = true;
	while(cont) {
		cont = false;
		for(auto& i : m_pathVariables) {
			auto it = p.find(i.second.string());
			if(it != std::string::npos) {
				cont = true;

				p = p.substr(0, it) + "$" + i.first + p.substr(it + i.second.string().length());
			}
		}
	}

	return p;
}


}
