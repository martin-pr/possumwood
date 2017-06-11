#include "filename.h"

Filename::Filename(std::initializer_list<std::string> extensions) : m_extensions(extensions) {

}

const boost::filesystem::path& Filename::filename() const {
	return m_filename;
}

void Filename::setFilename(const boost::filesystem::path& filename) {
	m_filename = filename;
}

const std::set<std::string>& Filename::extensions() const {
	return m_extensions;
}

bool Filename::operator == (const Filename& fn) const {
	return m_filename == fn.m_filename && m_extensions == fn.m_extensions;
}

bool Filename::operator != (const Filename& fn) const {
	return m_filename != fn.m_filename || m_extensions != fn.m_extensions;
}
