#pragma once

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/signals2.hpp>

#include <dependency_graph/graph.h>

#include "config.h"
#include "undo_stack.h"
#include "index.h"

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

/// App is a singleton, instantiated explicitly in main.cpp.
/// Holds global data about the application.
class App : public AppCore {
	public:
		static App& instance();

		App();
		~App();

		const boost::filesystem::path& filename() const;

		void newFile();
		void loadFile(const boost::filesystem::path& fn);
		void saveFile();
		void saveFile(const boost::filesystem::path& fn);

		QMainWindow* mainWindow() const;
		void setMainWindow(QMainWindow* win);

		void setTime(float time);
		float time() const;
		boost::signals2::connection onTimeChanged(std::function<void(float)> fn);

		Config& sceneConfig();

	private:
		static App* s_instance;

		boost::filesystem::path m_filename;

		QMainWindow* m_mainWindow;

		float m_time;
		boost::signals2::signal<void(float)> m_timeChanged;

		Config m_sceneConfig;
};

}
