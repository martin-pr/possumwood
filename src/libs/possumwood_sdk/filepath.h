#pragma once

#include <boost/filesystem/path.hpp>

namespace possumwood {

/// File path representation in the core of Possumwood.
/// Consists of a "prefix", which is a variable with procedurally determined path (e.g., $NODES or $PLUGINS)
/// and a relative path. Prefix can be empty - in that case, the path is an absolute path to a file.
/// Conversions to a path are only necessary for file-browsing UIs - any actual file access should be
/// implemented through IFilesystem abstraction to allow for mocking.
class Filepath {
  public:
	Filepath() = default;

	/// string methods always provide "short" version of the path, and will contain variables
	static Filepath fromString(const std::string& s);
	std::string toString() const;

	/// path methods always provide "long" version of the path, and can be used to open files
	static Filepath fromPath(const boost::filesystem::path& path);
	boost::filesystem::path toPath() const;

	bool empty() const;

	bool operator==(const Filepath& p) const;
	bool operator!=(const Filepath& p) const;

	std::string base;
	boost::filesystem::path relativePath;

  private:
	Filepath(const std::string& _base, const boost::filesystem::path& relative);

	friend class IFilesystem;
};

std::ostream& operator<<(std::ostream& out, const Filepath& path);

}  // namespace possumwood
