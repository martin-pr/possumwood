#include "app.h"

namespace possumwood {

AppCore* AppCore::s_instance = NULL;

AppCore::AppCore() {
	assert(s_instance == nullptr);
	s_instance = this;
}

AppCore::~AppCore() {
	assert(s_instance == this);
	s_instance = nullptr;
}

AppCore& AppCore::instance() {
	assert(s_instance != nullptr);
	return *s_instance;
}

dependency_graph::Graph& AppCore::graph() {
	return m_graph;
}

UndoStack& AppCore::undoStack() {
	return m_undoStack;
}

}
