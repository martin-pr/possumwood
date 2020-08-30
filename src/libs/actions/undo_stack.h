#pragma once

#include <dependency_graph/state.h>

#include <boost/noncopyable.hpp>
#include <functional>
#include <string>
#include <vector>

namespace possumwood {

/// A simple implementation of undo stack based on Command design pattern.
/// Each command uses a vector of std::functors to store the actual code
/// to be executed on each undo and redo.
class UndoStack : public boost::noncopyable {
  public:
	UndoStack();

	/// Action implementation - stores a stack of "things to be done".
	/// These are not executed on construction, but only after pushing onto
	/// a stack. Each action is atomic - if an exception is thrown while
	/// executing an action, all commands of a failed action are rolled back.
	class Action {
	  public:
		void addCommand(const std::string& name, const std::function<void()>& redo, const std::function<void()>& undo);

		/// appends all commands from action 'a' to this action
		void append(const Action& a);

	  private:
		struct Data {
			std::string name;
			std::function<void()> fn;
		};

		std::vector<Data> m_redo, m_undo;

		/// the actual implementation is handled in UndoStack code.
		friend class UndoStack;
		friend std::ostream& operator<<(std::ostream& out, const UndoStack::Action& action);
	};

	/// Executes an action and puts it to the undo stack to make it undoable.
	/// Any potential exceptions thrown from an action are handled (assuming
	/// they are based on std::exception) - with haltOnError=true, any error during a command will
	/// assume that command has not been finished (and any remaining state
	/// is rolled back), and all previously finished commands of an action
	/// needs rolling back. Exception is then forwared to the caller.
	/// When haltOnError is set to false, any errorring action is simply skipped,
	/// with the execution continuing (undo stack will not contain this failing action).
	/// Useful for actions that are expected to sometimes partially fail, like file loading.
	dependency_graph::State execute(const Action& action, bool haltOnError = true);

	/// Undoes last succesfully executed action. Cannot throw - action worked
	/// last time, so it should work this time as well.
	void undo();
	/// Redoes last undone action. The redo queue gets cleared when a new action
	/// is inserted onto the stack.
	void redo();

	std::size_t undoActionCount() const;
	std::size_t redoActionCount() const;
	bool empty() const;

	void clear();

  protected:
  private:
	std::vector<Action> m_undoStack, m_redoStack;

#ifndef NDEBUG
	bool m_executionInProgress;
#endif
};

std::ostream& operator<<(std::ostream& out, const UndoStack::Action& action);

}  // namespace possumwood
