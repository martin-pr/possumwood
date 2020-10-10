#include "filesystem_mock.h"

#include <boost/iostreams/stream.hpp>

namespace possumwood {

namespace {

/// a "source" device stream - wraps a single string into a stream
struct Source {
	typedef char char_type;
	typedef boost::iostreams::seekable_device_tag category;

	Source(const std::string& d) : data(d), offset(0) {
	}

	std::streamsize read(char* s, std::streamsize n) {
		if(offset == data.length())
			return -1;

		if(data.length() >= n + offset) {
			std::copy(data.data() + offset, data.data() + offset + n, s);
			offset += n;
			return n;
		}

		std::copy(data.data() + offset, data.data() + data.length(), s);

		std::streamsize result = data.length() - offset;
		offset = data.length();
		return result;
	}

	std::streamsize write(const char*, std::streamsize n) {
		throw std::runtime_error("Attempting to write to a read-only buffer.");
		return 0;
	}

	boost::iostreams::stream_offset seek(boost::iostreams::stream_offset /*off*/, std::ios_base::seekdir /*way*/) {
		return 0;
	}

	const std::string& data;
	std::size_t offset;
};

/// a "sink" buffer (not a full device) for writing into a string
struct Sink {
	typedef char char_type;
	typedef boost::iostreams::sink_tag category;

	Sink(std::string& d) : data(d) {
	}

	std::streamsize write(const char* s, std::streamsize n) {
		data += std::string(s, n);
		return n;
	}

	std::string& data;
};

}  // namespace

FilesystemMock::FilesystemMock() {
}

void FilesystemMock::addFile(const Filepath& path, const std::string& content) {
	m_content[path] = content;
}

std::unique_ptr<std::istream> FilesystemMock::read(const Filepath& path) {
	return std::unique_ptr<std::istream>(new boost::iostreams::stream<Source>(m_content[path]));
}

std::unique_ptr<std::ostream> FilesystemMock::write(const Filepath& path) {
	m_content[path].clear();
	return std::unique_ptr<std::ostream>(new boost::iostreams::stream<Sink>(m_content[path]));
}

bool FilesystemMock::exists(const Filepath& path) const {
	return m_content.find(path) != m_content.end();
}

boost::filesystem::path FilesystemMock::expandPath(const Filepath& path) const {
	return path.relativePath;
}

Filepath FilesystemMock::shrinkPath(const boost::filesystem::path& path) const {
	return makeFilepath("", path);
}

}  // namespace possumwood
