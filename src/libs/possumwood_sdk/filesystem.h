#pragma once

#include <map>
#include <string>

#include <boost/filesystem/path.hpp>

namespace possumwood {

class IFilesystem {
  public:
	virtual ~IFilesystem(){};

	virtual boost::filesystem::path expandPath(const boost::filesystem::path& path) const = 0;
	virtual boost::filesystem::path shrinkPath(const boost::filesystem::path& path) const = 0;

  private:
};

class Filesystem : public IFilesystem {
  public:
	Filesystem();

	boost::filesystem::path expandPath(const boost::filesystem::path& path) const override;
	boost::filesystem::path shrinkPath(const boost::filesystem::path& path) const override;

  private:
	std::map<std::string, boost::filesystem::path> m_pathVariables;
};

}  // namespace possumwood
