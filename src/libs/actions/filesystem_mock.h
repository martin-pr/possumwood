#pragma once

#include "filesystem.h"

namespace possumwood {

class FilesystemMock : public IFilesystem {
  public:
	FilesystemMock();

	void addFile(const Filepath& path, const std::string& content);

	std::unique_ptr<std::istream> read(const Filepath& path) override;
	std::unique_ptr<std::ostream> write(const Filepath& path) override;
	bool exists(const Filepath& path) const override;

  protected:
	boost::filesystem::path expandPath(const Filepath& path) const override;
	Filepath shrinkPath(const boost::filesystem::path& path) const override;

  private:
	std::map<Filepath, std::string> m_content;
};

}  // namespace possumwood
