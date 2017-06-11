#include "app.h"

#include <cassert>
#include <fstream>

#include <dependency_graph/io/graph.h>

#include "io/node_data.h"

namespace possumwood {

App* App::s_instance = NULL;

App::App() : m_mainWindow(NULL) {
	assert(s_instance == nullptr);
	s_instance = this;
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
	std::ifstream in(filename.string());

	dependency_graph::io::json json;
	in >> json;

	dependency_graph::io::adl_serializer<dependency_graph::Graph>::from_json(json, m_graph);

	m_filename = filename;
}

void App::saveFile() {
	assert(!m_filename.empty());
	saveFile(m_filename);
}

void App::saveFile(const boost::filesystem::path& fn) {
	std::ofstream out(fn.string());

	dependency_graph::io::json json;
	json = m_graph;

	out << std::setw(4) << json;

	m_filename = fn;
}

QMainWindow* App::mainWindow() const {
	return m_mainWindow;
}

void App::setMainWindow(QMainWindow* win) {
	assert(m_mainWindow == NULL && "setMainWindow is called only once at the beginning of an application");
	m_mainWindow = win;
}

}
