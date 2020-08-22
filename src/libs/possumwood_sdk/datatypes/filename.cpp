#include "filename.h"

#include <possumwood_sdk/app.h>

namespace possumwood {

Filename::Filename(std::initializer_list<std::string> extensions) : m_extensions(extensions) {
}

Filename::Filename(const Filename& fn) : m_filename(fn.m_filename), m_extensions(fn.m_extensions) {
}

const boost::filesystem::path Filename::filename(bool makeAbsolute) const {
	if(!makeAbsolute)
		return m_filename;
	else
		return App::instance().filesystem().expandPath(m_filename);
}

void Filename::setFilename(const boost::filesystem::path& filename) {
	m_filename = App::instance().filesystem().shrinkPath(filename);
}

const std::set<std::string>& Filename::extensions() const {
	return m_extensions;
}

Filename& Filename::operator=(const Filename& fn) {
	// only assign a value if the m_extension array is empty
	// -> allows to keep the extensions list while allowing to change
	//    the filename value in the UI / serialization
	if(m_extensions.empty())
		m_extensions = fn.m_extensions;

	m_filename = fn.m_filename;

	return *this;
}

bool Filename::operator==(const Filename& fn) const {
	return m_filename == fn.m_filename && m_extensions == fn.m_extensions;
}

bool Filename::operator!=(const Filename& fn) const {
	return m_filename != fn.m_filename || m_extensions != fn.m_extensions;
}

std::ostream& operator<<(std::ostream& out, const Filename& f) {
	out << f.filename();
	return out;
}

/////////////////

namespace {

void toJson(::possumwood::io::json& json, const Filename& value) {
	json = value.filename(false).string();
}

void fromJson(const ::possumwood::io::json& json, Filename& value) {
	value.setFilename(json.get<std::string>());
}

}  // namespace

IO<Filename> Traits<Filename>::io(&toJson, &fromJson);

}  // namespace possumwood
