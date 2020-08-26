#include "filepath.h"

#include "possumwood_sdk/app.h"

namespace possumwood {

Filepath Filepath::fromString(const std::string& s) {
	if(s.empty())
		return Filepath("", "");

	// $ on shortened path is allowed only at the beginning - this mechanism doesn't expand generic env variables
	if(s[0] == '$') {
		auto slash = s.find('/');
		if(slash == std::string::npos)
			return Filepath(s.substr(1), boost::filesystem::path());
		return Filepath(s.substr(1, slash - 1), s.substr(slash + 1));
	}

	return Filepath("", s);
}

std::string Filepath::toString() const {
	if(base.empty())
		return relativePath.string();
	return "$" + base + "/" + relativePath.string();
}

/// path methods always provide "long" version of the path, and can be used to open files
Filepath Filepath::fromPath(const boost::filesystem::path& path) {
	return App::instance().filesystem().shrinkPath(path);
}

boost::filesystem::path Filepath::toPath() const {
	return App::instance().filesystem().expandPath(*this);
}

bool Filepath::empty() const {
	return base.empty() && relativePath.empty();
}

bool Filepath::operator==(const Filepath& p) const {
	return base == p.base && relativePath == p.relativePath;
}

bool Filepath::operator!=(const Filepath& p) const {
	return base != p.base || relativePath != p.relativePath;
}

bool Filepath::operator<(const Filepath& p) const {
	if(base != p.base)
		return base < p.base;
	return relativePath < p.relativePath;
}

Filepath::Filepath(const std::string& _base, const boost::filesystem::path& relative)
    : base(_base), relativePath(relative) {
}

std::ostream& operator<<(std::ostream& out, const Filepath& path) {
	out << path.base << "/" << path.relativePath.string();

	return out;
}

}  // namespace possumwood
