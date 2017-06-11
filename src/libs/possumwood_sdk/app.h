#pragma once

#include <boost/noncopyable.hpp>

namespace possumwood {

/// App is a singleton, instantiated explicitly in main.cpp.
/// Holds global data about the application.
class App : public boost::noncopyable {
	public:
		static App& instance();

		App();
		~App();

	protected:
	private:
		static App* s_instance;
};

}
