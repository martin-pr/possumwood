#pragma once

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/signals2.hpp>

#include <dependency_graph/graph.h>

#include "undo_stack.h"

class QMainWindow;

namespace possumwood {

/// AppBase is an explicitly-instantiated singleton object, allowing
/// to mock the undo stack without the need to explicitly instantiate
/// the UI classes.
class AppCore : public boost::noncopyable {
	public:
		static AppCore& instance();

		AppCore();
		virtual ~AppCore();

		dependency_graph::Graph& graph();
		UndoStack& undoStack();

	private:
		static AppCore* s_instance;

		dependency_graph::Graph m_graph;
		UndoStack m_undoStack;
};

}
