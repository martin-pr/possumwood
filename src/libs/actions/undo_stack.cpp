#include "undo_stack.h"

#include <cassert>
#include <iostream>
#include <sstream>

#include <boost/noncopyable.hpp>

namespace possumwood {

void UndoStack::Action::addCommand(const std::string& name, const std::function<void()>& redo, const std::function<void()>& undo) {
	assert(undo);
	assert(redo);

	m_undo.push_back(Data{name, undo});
	m_redo.push_back(Data{name, redo});
}

void UndoStack::Action::append(const Action& a) {
	for(auto& c : a.m_undo)
		m_undo.push_back(c);

	for(auto& c : a.m_redo)
		m_redo.push_back(c);
}

//////////////////////////

UndoStack::UndoStack()
#ifndef NDEBUG
: m_executionInProgress(false)
#endif
{
}

dependency_graph::State UndoStack::execute(const Action& input_action, bool haltOnError) {
	// any action executed in this manner has to have m_undo and m_redo the same size
	assert(input_action.m_redo.size() == input_action.m_undo.size());

	dependency_graph::State state;
	// construct an action to be pushed on the stack - when haltOnError is false, this action might not contain all
	// undo/redo commands the input action does, as some parts might have errored and will now be skipped on next evaluation.
	Action action;

#ifndef NDEBUG
	assert(!m_executionInProgress);

	m_executionInProgress = true;
#endif

	if(!input_action.m_undo.empty()) {
		assert(input_action.m_undo.size() == input_action.m_redo.size());

		// first, execute all commands in the undo part of the new action
		for(std::size_t counter = 0; counter < input_action.m_redo.size(); ++counter) {
			action.m_redo.push_back(input_action.m_redo[counter]);
			action.m_undo.push_back(input_action.m_undo[counter]);

			try {
				// just run the command
				input_action.m_redo[counter].fn();
			}
			catch(const std::exception& err) {
				std::stringstream ss;
				ss << "Exception while running '" << input_action.m_redo[counter].name <<"': " << err.what();

				if(haltOnError) {
					// an exception was caught during the last command - undo all previous commands
					std::size_t rollback = counter;
					while(rollback > 0) {
						--rollback;

						input_action.m_undo[rollback].fn();
					}

#ifndef NDEBUG
					m_executionInProgress = false;
#endif

					// and rethrow the exception
					throw std::runtime_error(ss.str());
				}

				else
					state.addError(ss.str());

				action.m_undo.pop_back();
				action.m_redo.pop_back();
			}
			catch(...) {
				std::stringstream ss;
				ss << "Unhandled exception while running '" << input_action.m_redo[counter].name <<"'.";

				if(haltOnError) {
					// an exception was caught during the last command - undo all previous commands
					while(counter > 0) {
						--counter;

						input_action.m_undo[counter].fn();
					}

#ifndef NDEBUG
					m_executionInProgress = false;
#endif

					// and rethrow the exception
					throw std::runtime_error(ss.str());
				}

				else
					state.addError(ss.str());

				action.m_undo.pop_back();
				action.m_redo.pop_back();
			}
		}

		// if no exception was thrown during the execution, add this command to the undo stack
		m_undoStack.push_back(action);
		m_redoStack.clear();
	}

#ifndef NDEBUG
	m_executionInProgress = false;
#endif

	return state;
}

void UndoStack::undo() {
#ifndef NDEBUG
	assert(!m_executionInProgress);

	m_executionInProgress = true;
#endif

	// execute the last undo queue item
	if(!m_undoStack.empty()) {
		for(std::vector<UndoStack::Action::Data>::const_reverse_iterator it = m_undoStack.back().m_undo.rbegin(); it != m_undoStack.back().m_undo.rend(); ++it)
			it->fn();

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
		for(std::vector<Action::Data>::const_iterator it = m_redoStack.back().m_redo.begin(); it != m_redoStack.back().m_redo.end(); ++it)
			it->fn();

		// and move it to the undo stack
		m_undoStack.push_back(m_redoStack.back());
		m_redoStack.pop_back();
	}

#ifndef NDEBUG
	m_executionInProgress = false;
#endif
}

std::size_t UndoStack::undoActionCount() const {
	return m_undoStack.size();
}

std::size_t UndoStack::redoActionCount() const {
	return m_redoStack.size();
}

bool UndoStack::empty() const {
	return m_undoStack.empty() && m_redoStack.empty();
}

void UndoStack::clear() {
	m_undoStack.clear();
	m_redoStack.clear();
}

}
