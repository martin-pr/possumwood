#include "app.h"

#include <cassert>
#include <fstream>

#include <boost/filesystem.hpp>

#include <QMainWindow>

#include <dependency_graph/io/graph.h>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include "node_data.h"
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
}

App::~App() {
	assert(s_instance == this);
	s_instance = nullptr;
}

App& App::instance() {
	assert(s_instance != nullptr);
	return *s_instance;
}

dependency_graph::Graph& App::graph() {
	return m_graph;
}

const boost::filesystem::path& App::filename() const {
	return m_filename;
}

void App::newFile() {
	m_graph.clear();
	m_filename = "";
}

void App::loadFile(const boost::filesystem::path& filename) {
	if(!boost::filesystem::exists(filename))
		throw std::runtime_error("Cannot open " + filename.string() +
		                         " - file not found.");

	// read the json file
	std::ifstream in(filename.string());
	dependency_graph::io::json json;
	in >> json;

	// update the opened filename
	const boost::filesystem::path oldFilename = m_filename;
	m_filename = filename;

	try {
		// read the graph
		m_graph.clear();
		dependency_graph::io::adl_serializer<dependency_graph::Graph>::from_json(json,
		                                                                         m_graph);

		// and read the scene config
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

		// read the UI configuration
		if(json.find("ui_geometry") != json.end())
			mainWindow()->restoreGeometry(
			    QByteArray::fromBase64(json["ui_geometry"].get<std::string>().c_str()));
		if(json.find("ui_state") != json.end())
			mainWindow()->restoreState(
			    QByteArray::fromBase64(json["ui_state"].get<std::string>().c_str()));
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

void App::saveFile(const boost::filesystem::path& fn) {
	// make a json instance containing the graph
	dependency_graph::io::json json;
	json = m_graph;

	// save scene config into the json object
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

	// and save the UI configuration
	json["ui_geometry"] = mainWindow()->saveGeometry().toBase64().data();
	json["ui_state"] = mainWindow()->saveState().toBase64().data();

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

void App::setTime(float time) {
	if(m_time != time) {
		m_time = time;
		m_timeChanged(time);

		// TERRIBLE HACK - a special node type that outputs time is handled here
		for(auto& n : graph().nodes())
			if(n.metadata().metadata().type() == "time")
				n.port(0).set<float>(time);
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

UndoStack& App::undoStack() {
	return m_undoStack;
}

Index& App::index() {
	return m_index;
}
}
