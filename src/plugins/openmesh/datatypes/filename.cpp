#include "filename.h"

#include <possumwood_sdk/app.h>

namespace possumwood {

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

Filename& Filename::operator = (const Filename& fn) {
	// only assign a value if the m_extension array is empty
	// -> allows to keep the extensions list while allowing to change
	//    the filename value in the UI / serialization
	if(m_extensions.empty())
		m_extensions = fn.m_extensions;

	m_filename = fn.m_filename;

	return *this;
}

}
