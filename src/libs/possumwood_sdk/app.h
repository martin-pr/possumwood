#pragma once

#include <dependency_graph/graph.h>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "actions/app.h"
#include "actions/filepath.h"
#include "actions/filesystem.h"
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

	/// App initialises with a mockable filesystem instance. It is shared to enable it being used
	/// in tests.
	App(std::shared_ptr<IFilesystem> filesystem = std::make_shared<Filesystem>());
	~App();

	const Filepath& filename() const;

	void newFile();
	dependency_graph::State loadFile(const Filepath& fn, bool alterCurrentFilename = true);
	void saveFile();
	void saveFile(const Filepath& fn, bool saveSceneConfig = true);

	QMainWindow* mainWindow() const;
	void setMainWindow(QMainWindow* win);

	/// run OpenGL drawing, by iterating over all Node instances and calling any existing Drawable::doDraw()
	void draw(const possumwood::ViewportState& viewport,
	          std::function<void(const dependency_graph::NodeBase&)> stateChangedCallback =
	              std::function<void(const dependency_graph::NodeBase&)>());

	void setTime(float time);
	float time() const;
	boost::signals2::connection onTimeChanged(std::function<void(float)> fn);

	Config& sceneConfig();
	Description& sceneDescription();

  private:
	dependency_graph::State loadFile(const nlohmann::json& json);
	void saveFile(nlohmann::json& json, bool saveSceneConfig = true);

	static App* s_instance;

	Filepath m_filename;

	QMainWindow* m_mainWindow;

	float m_time;
	boost::signals2::signal<void(float)> m_timeChanged;

	Config m_sceneConfig;
	Description m_sceneDescription;
};

}  // namespace possumwood
