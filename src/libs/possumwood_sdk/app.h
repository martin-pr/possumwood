#pragma once

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

#include <dependency_graph/graph.h>

class QMainWindow;

namespace possumwood {

/// App is a singleton, instantiated explicitly in main.cpp.
/// Holds global data about the application.
class App : public boost::noncopyable {
	public:
		static App& instance();

		App();
		~App();

		dependency_graph::Graph& graph();

		const boost::filesystem::path& filename() const;

		void newFile();
		void loadFile(const boost::filesystem::path& fn);
		void saveFile();
		void saveFile(const boost::filesystem::path& fn);

		QMainWindow* mainWindow() const;
		void setMainWindow(QMainWindow* win);

	protected:
	private:
		static App* s_instance;

		dependency_graph::Graph m_graph;
		boost::filesystem::path m_filename;

		QMainWindow* m_mainWindow;
};

}
