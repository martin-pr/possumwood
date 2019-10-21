#pragma once

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/signals2.hpp>

#include <dependency_graph/graph.h>

#include "actions/app.h"
#include "actions/io.h"

#include "config.h"
#include "description.h"
#include "viewport_state.h"

class QMainWindow;

namespace possumwood {

/// App is a singleton, instantiated explicitly in main.cpp.
/// Holds global data about the application.
class App : public AppCore {
	public:
		static App& instance();

		App();
		~App();

		const boost::filesystem::path& filename() const;

		void newFile();
		dependency_graph::State loadFile(const boost::filesystem::path& fn, bool alterCurrentFilename = true);
		dependency_graph::State loadFile(const possumwood::io::json& json);
		void saveFile();
		void saveFile(const boost::filesystem::path& fn);
		void saveFile(possumwood::io::json& json, bool saveSceneConfig = true);

		QMainWindow* mainWindow() const;
		void setMainWindow(QMainWindow* win);

		/// run OpenGL drawing, by iterating over all Node instances and calling any existing Drawable::doDraw()
		void draw(const possumwood::ViewportState& viewport, std::function<void(const dependency_graph::NodeBase&)> stateChangedCallback = std::function<void(const dependency_graph::NodeBase&)>());

		void setTime(float time);
		float time() const;
		boost::signals2::connection onTimeChanged(std::function<void(float)> fn);

		Config& sceneConfig();
		Description& sceneDescription();

		boost::filesystem::path expandPath(const boost::filesystem::path& path) const;
		boost::filesystem::path shrinkPath(const boost::filesystem::path& path) const;

	private:
		static App* s_instance;

		boost::filesystem::path m_filename;

		QMainWindow* m_mainWindow;

		float m_time;
		boost::signals2::signal<void(float)> m_timeChanged;

		Config m_sceneConfig;
		Description m_sceneDescription;
		std::map<std::string, boost::filesystem::path> m_pathVariables;
};

}
