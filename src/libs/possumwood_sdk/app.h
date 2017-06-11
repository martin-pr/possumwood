#pragma once

#include <boost/noncopyable.hpp>

#include <dependency_graph/graph.h>

namespace possumwood {

/// App is a singleton, instantiated explicitly in main.cpp.
/// Holds global data about the application.
class App : public boost::noncopyable {
	public:
		static App& instance();

		App();
		~App();

		dependency_graph::Graph& graph();

	protected:
	private:
		static App* s_instance;

		dependency_graph::Graph m_graph;
};

}
