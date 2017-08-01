#include "undo_stack.h"

#include <cassert>
#include <iostream>

#include <boost/noncopyable.hpp>

namespace possumwood {

void UndoStack::Action::addCommand(const std::function<void()>& redo, const std::function<void()>& undo) {
	assert(undo);
	assert(redo);

	m_undo.push_back(undo);
	m_redo.push_back(redo);
}

void UndoStack::Action::append(const Action& a) {
	for(auto& c : a.m_undo)
		m_undo.push_back(c);

	for(auto& c : a.m_redo)
		m_redo.push_back(c);
}

UndoStack::UndoStack()
#ifndef NDEBUG
: m_executionInProgress(false)
#endif
{
}

void UndoStack::execute(const Action& action) {
#ifndef NDEBUG
	assert(!m_executionInProgress);

	m_executionInProgress = true;
#endif

	if(!action.m_undo.empty()) {
		assert(action.m_undo.size() == action.m_redo.size());

		// first, execute all commands in the undo part of the new action
		for(std::size_t counter = 0; counter < action.m_redo.size(); ++counter) {
			try {
				// just run the command
				action.m_redo[counter]();
			}
			catch(...) {
				// an exception was caught during the last command - undo all previous commands
				while(counter > 0) {
					--counter;

					action.m_undo[counter]();
				}

				// and rethrow the exception
				throw;
			}
		}

		// if no exception was thrown during the execution, add this command to the undo stack
		m_undoStack.push_back(action);
		m_redoStack.clear();
	}

#ifndef NDEBUG
	m_executionInProgress = false;
#endif
}

void UndoStack::undo() {
#ifndef NDEBUG
	assert(!m_executionInProgress);

	m_executionInProgress = true;
#endif

	// execute the last undo queue item
	if(!m_undoStack.empty()) {
		for(std::vector<std::function<void()>>::const_reverse_iterator it = m_undoStack.back().m_undo.rbegin(); it != m_undoStack.back().m_undo.rend(); ++it)
			(*it)();

		// and move it to the redo stack
		m_redoStack.push_back(m_undoStack.back());
		m_undoStack.pop_back();
	}

#ifndef NDEBUG
	m_executionInProgress = false;
#endif
}

void UndoStack::redo() {
#ifndef NDEBUG
	assert(!m_executionInProgress);

	m_executionInProgress = true;
#endif

	// execute the last redo queue item
	if(!m_redoStack.empty()) {
		for(std::vector<std::function<void()>>::const_iterator it = m_redoStack.back().m_redo.begin(); it != m_redoStack.back().m_redo.end(); ++it)
			(*it)();

		// and move it to the undo stack
		m_undoStack.push_back(m_redoStack.back());
		m_redoStack.pop_back();
	}

#ifndef NDEBUG
	m_executionInProgress = false;
#endif
}

}
