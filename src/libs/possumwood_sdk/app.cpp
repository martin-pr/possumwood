#include "app.h"

#include <cassert>

namespace possumwood {

App* App::s_instance = NULL;

App::App() {
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

}
