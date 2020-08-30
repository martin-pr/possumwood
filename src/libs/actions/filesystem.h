#pragma once

#include <boost/filesystem/path.hpp>
#include <map>
#include <string>

#include "filepath.h"

namespace possumwood {

/// A mockable filesystem access interface.
/// Allows for different implementation of file access, and for conditional implementation of specific file access types
/// (e.g. plugins).
class IFilesystem {
  public:
	virtual ~IFilesystem(){};

	virtual std::unique_ptr<std::istream> read(const Filepath& path) = 0;
	virtual std::unique_ptr<std::ostream> write(const Filepath& path) = 0;
	virtual bool exists(const Filepath& path) const = 0;

  protected:
	static Filepath makeFilepath(const std::string& base, const boost::filesystem::path& path);

	// TODO: should these be moved to Filepath? No real need to abstract them like this.
	virtual boost::filesystem::path expandPath(const Filepath& path) const = 0;
	virtual Filepath shrinkPath(const boost::filesystem::path& path) const = 0;

	friend class Filepath;
};

class Filesystem : public IFilesystem {
  public:
	Filesystem();

	std::unique_ptr<std::istream> read(const Filepath& path) override;
	std::unique_ptr<std::ostream> write(const Filepath& path) override;
	bool exists(const Filepath& path) const override;

  protected:
	boost::filesystem::path expandPath(const Filepath& path) const override;
	Filepath shrinkPath(const boost::filesystem::path& path) const override;

  private:
	std::map<std::string, boost::filesystem::path> m_pathVariables;
};

}  // namespace possumwood
