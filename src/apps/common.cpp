#include "common.h"

#include <dlfcn.h>

#include <iostream>

#include <boost/filesystem.hpp>

#include <possumwood_sdk/app.h>

namespace fs = boost::filesystem;

PluginsRAII::PluginsRAII() {
	// scan for plugins
	for(fs::directory_iterator itr(possumwood::Filepath::fromString("$PLUGINS").toPath());
	    itr != fs::directory_iterator(); ++itr) {
		if(fs::is_regular_file(itr->status()) && itr->path().extension() == ".so") {
			std::cout << "Loading plugin " << itr->path().string() << " ... " << std::flush;

			void* ptr = dlopen(itr->path().string().c_str(), RTLD_NOW);
			if(ptr) {
				m_pluginHandles.push_back(std::make_pair(ptr, itr->path().string()));
				std::cout << "done." << std::endl;
			}
			else
				std::cout << dlerror() << std::endl;
		}
	}
}

PluginsRAII::~PluginsRAII() {
	// unload all plugins
	while(!m_pluginHandles.empty()) {
		dlclose(m_pluginHandles.back().first);
		m_pluginHandles.pop_back();
	}
}
