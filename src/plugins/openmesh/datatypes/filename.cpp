#include "filename.h"

#include <possumwood_sdk/app.h>

Filename::Filename(std::initializer_list<std::string> extensions) : m_extensions(extensions) {

}

const boost::filesystem::path Filename::filename(bool makeAbsolute) const {
	// make the filename absolute
	if(makeAbsolute) {
		if(m_filename.is_absolute())
			return m_filename;
		else
			return possumwood::App::instance().filename().parent_path() / m_filename;
	}

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
