#include "app.h"

namespace possumwood {

AppCore* AppCore::s_instance = NULL;

AppCore::AppCore(std::shared_ptr<IFilesystem> filesystem) : m_filesystem(filesystem) {
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

IFilesystem& AppCore::filesystem() {
	return *m_filesystem;
}

}  // namespace possumwood
