#pragma once

#include <vector>
#include <string>

/// A RAII class loading plugins (assumes an App instance already exists).
/// Will unload plugins on destruction.
class PluginsRAII {
	public:
		PluginsRAII();
		~PluginsRAII();

	private:
		PluginsRAII(const PluginsRAII&) = delete;
		PluginsRAII& operator = (const PluginsRAII&) = delete;

		std::vector<std::pair<void*, std::string>> m_pluginHandles;
};
