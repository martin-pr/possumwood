#include "app.h"

#include <cassert>
#include <fstream>
#include <regex>

#include <boost/filesystem.hpp>

#include <QApplication>
#include <QMainWindow>

#include <dependency_graph/node.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes_iterator.inl>
#include <dependency_graph/port.inl>

#include <actions/actions.h>

#include <possumwood_sdk/gl.h>
#include <possumwood_sdk/metadata.h>

#include "config.inl"

namespace possumwood {

namespace {

boost::filesystem::path expandEnvvars(const boost::filesystem::path& p) {
	boost::filesystem::path result = p;

	bool expanded = true;
	while(expanded) {
		expanded = false;

		for(auto it = result.begin(); it != result.end(); ++it) {
			const std::string part = it->string();
			if(!part.empty() && part[0] == '$') {
				const char* envvar = getenv(part.substr(1).c_str());
				if(envvar != nullptr) {
					boost::filesystem::path tmp;
					for(auto it2 = result.begin(); it2 != result.end(); ++it2)
						if(it2 != it)
							tmp /= *it2;
						else
							tmp /= envvar;
					result = tmp;

					expanded = true;
					break;
				}
			}
		}
	}

	return result;
}

}  // namespace

App* App::s_instance = NULL;

App::App(std::unique_ptr<IFilesystem> filesystem)
    : m_mainWindow(NULL), m_time(0.0f), m_filesystem(std::move(filesystem)) {
	assert(s_instance == nullptr);
	s_instance = this;

	////////////////////////
	// scene configuration

	m_sceneConfig.addItem(
	    Config::Item("start_time", "timeline", 0.0f, Config::Item::kNoFlags, "Start of the timeline (seconds)"));

	m_sceneConfig.addItem(
	    Config::Item("end_time", "timeline", 5.0f, Config::Item::kNoFlags, "End of the timeline (seconds)"));

	m_sceneConfig.addItem(
	    Config::Item("fps", "timeline", 24.0f, Config::Item::kNoFlags, "Scene's frames-per-second value"));
}

App::~App() {
	m_timeChanged.disconnect_all_slots();

	assert(s_instance == this);
	s_instance = nullptr;
}

App& App::instance() {
	assert(s_instance != nullptr);
	return *s_instance;
}

const Filepath& App::filename() const {
	return m_filename;
}

void App::newFile() {
	graph().clear();
	m_filename = Filepath();
	m_sceneDescription.clear();
}

dependency_graph::State App::loadFile(const possumwood::io::json& json) {
	// read the graph
	graph().clear();
	undoStack().clear();
	m_sceneDescription.clear();

	dependency_graph::Selection selection;  // throwaway

	dependency_graph::State state;

	try {
		state = possumwood::actions::fromJson(graph(), selection, json, false);

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

		if(json.find("description") != json.end())
			m_sceneDescription.deserialize(json["description"].get<std::string>());

		// read the UI configuration
		if(QCoreApplication::instance() != nullptr) {
			if(json.find("ui_geometry") != json.end())
				mainWindow()->restoreGeometry(QByteArray::fromBase64(json["ui_geometry"].get<std::string>().c_str()));
			if(json.find("ui_state") != json.end())
				mainWindow()->restoreState(QByteArray::fromBase64(json["ui_state"].get<std::string>().c_str()));
		}
	}
	catch(std::exception& exc) {
		state.addError(exc.what());
	}

	return state;
}

dependency_graph::State App::loadFile(const Filepath& filename, bool alterCurrentFilename) {
	if(!m_filesystem->exists(filename))
		throw std::runtime_error("Cannot open " + filename.toString() + " - file not found.");
	// read the json file
	auto in = m_filesystem->read(filename);
	possumwood::io::json json;
	(*in) >> json;

	// update the opened filename
	if(alterCurrentFilename)
		m_filename = filename;

	const dependency_graph::State state = loadFile(json);

	return state;
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

		json["description"] = m_sceneDescription.serialize();
	}

	// and save the UI configuration
	if(QCoreApplication::instance() != nullptr) {
		json["ui_geometry"] = mainWindow()->saveGeometry().toBase64().data();
		json["ui_state"] = mainWindow()->saveState().toBase64().data();
	}
}

void App::saveFile(const Filepath& fn) {
	possumwood::io::json json;
	saveFile(json);

	// save the json to the file
	auto out = m_filesystem->write(fn);
	(*out) << std::setw(4) << json;

	// and update the filename
	m_filename = fn;
}

QMainWindow* App::mainWindow() const {
	return m_mainWindow;
}

void App::setMainWindow(QMainWindow* win) {
	assert(m_mainWindow == NULL && "setMainWindow is called only once at the beginning of an application");
	m_mainWindow = win;
}

void App::draw(const possumwood::ViewportState& viewport,
               std::function<void(const dependency_graph::NodeBase&)> stateChangedCallback) {
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
		for(dependency_graph::Nodes::iterator i = graph().nodes().begin(dependency_graph::Nodes::kRecursive);
		    i != graph().nodes().end(); ++i) {
			if(i->metadata()->type() == "time")
				i->port(0).set<float>(time);
			if(i->metadata()->type() == "frame")
				i->port(0).set<unsigned>(time * possumwood::App::instance().sceneConfig()["fps"].as<float>());
		}
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

Description& App::sceneDescription() {
	return m_sceneDescription;
}

const IFilesystem& App::filesystem() const {
	return *m_filesystem;
}

}  // namespace possumwood
