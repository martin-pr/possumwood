#include "filesystem.h"

#include <fstream>
#include <iostream>
#include <regex>

#include <boost/filesystem.hpp>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace {

boost::filesystem::path expandEnvvars(const boost::filesystem::path& p) {
	boost::filesystem::path result = p;

	bool expanded = true;
	while(expanded) {
		expanded = false;

		for(auto it = result.begin(); it != result.end(); ++it) {
			const std::string part = it->string();
			if(!part.empty() && part[0] == '$') {
				const char* envvar = getenv(part.substr(1).c_str());
				if(envvar != nullptr) {
					boost::filesystem::path tmp;
					for(auto it2 = result.begin(); it2 != result.end(); ++it2)
						if(it2 != it)
							tmp /= *it2;
						else
							tmp /= envvar;
					result = tmp;

					expanded = true;
					break;
				}
			}
		}
	}

	return result;
}

}  // namespace

namespace possumwood {

Filesystem::Filesystem() {
	///////////////////////
	// resolving paths from possumwood.conf

	boost::filesystem::path conf_path;

	// conf path can be explicitly defined during the build (at which point we just use it)
#ifdef POSSUMWOOD_CONF_PATH
	conf_path = boost::filesystem::path(TOSTRING(POSSUMWOOD_CONF_PATH));

	conf_path = expandEnvvars(conf_path);

	// however, if that path doesn't exist (for development only)
	if(!boost::filesystem::exists(conf_path)) {
#endif

		// find the config file in the current or parent directories
		conf_path = boost::filesystem::path(boost::filesystem::current_path());
		while(!conf_path.empty()) {
			if(boost::filesystem::exists(conf_path / "possumwood.conf")) {
				conf_path = conf_path / "possumwood.conf";
				break;
			}
			else
				conf_path = conf_path.parent_path();
		}

#ifdef POSSUMWOOD_CONF_PATH
	}
#endif

	if(!conf_path.empty()) {
		// parent directory for the config file - used for resolving relative paths
		const boost::filesystem::path full_path = conf_path.parent_path();

		std::ifstream cfg(conf_path.string());
		while(!cfg.eof() && cfg.good()) {
			std::string line;
			std::getline(cfg, line);

			if(!line.empty()) {
				// read the key/value pair of key->path from the file
				std::string key;
				std::string value;

				int state = 0;
				for(std::size_t c = 0; c < line.length(); ++c) {
					if(state == 0) {
						if(line[c] != ' ' && line[c] != '\t')
							key.push_back(line[c]);
						else
							state = 1;
					}

					if(state == 1) {
						if(line[c] != ' ' && line[c] != '\t')
							state = 2;
					}

					if(state == 2)
						value.push_back(line[c]);
				}

				boost::filesystem::path path(value);

				// expand any env vars
				path = expandEnvvars(path);

				// resolve relative paths to absolute
				if(path.is_relative())
					path = full_path / path;

				// store this path variable
				m_pathVariables[key] = path.string();
			}
		}
	}
	else
		std::cout << "Warning: configuration file 'possumwood.conf' not found!" << std::endl;
}

boost::filesystem::path Filesystem::expandPath(const boost::filesystem::path& path) const {
	std::string p = path.string();

	const std::regex var("\\$[A-Z]+");
	std::smatch match;
	while(std::regex_search(p, match, var)) {
		auto it = m_pathVariables.find(match.str().substr(1));
		if(it != m_pathVariables.end())
			p = match.prefix().str() + it->second.string() + match.suffix().str();
		else
			break;
	}

	return p;
}

boost::filesystem::path Filesystem::shrinkPath(const boost::filesystem::path& path) const {
	std::string p = path.string();

	bool cont = true;
	while(cont) {
		cont = false;
		for(auto& i : m_pathVariables) {
			auto it = p.find(i.second.string());
			if(it != std::string::npos) {
				cont = true;

				p = p.substr(0, it) + "$" + i.first + p.substr(it + i.second.string().length());
			}
		}
	}

	return p;
}

}  // namespace possumwood
