#include "description.h"

#include <sstream>

namespace possumwood {

namespace {

bool starts_with(const std::string& str, const std::string& prefix) {
	if(str.length() < prefix.length())
		return false;

	auto it1 = str.begin();
	auto it2 = prefix.begin();

	while(it2 != prefix.end()) {
		if(*it1 != *it2)
			return false;

		++it1;
		++it2;
	}

	return true;
}

}

void Description::setMarkdown(const std::string& md) {
	m_markDown = md;
}

const std::string& Description::markdown() const {
	return m_markDown;
}

std::string Description::html() const {
	std::stringstream ss(m_markDown);

	std::string result;

	while(ss.good()) {
		std::string line;
		std::getline(ss, line);

		if(ss.good()) {
			if(starts_with(line, "# "))
				result += "<h1>" + line "</h1>\n";
			else if(starts_with(line, "## "))
				result += "<h2>" + line "</h2>\n";
			else if(starts_with(line, "### "))
				result += "<h3>" + line "</h3>\n";
			else if(starts_with(line, "#### "))
				result += "<h4>" + line "</h4>\n";
			else if(!line.empty())
				result += "<p>" + line "</p>\n";
		}
	}

	return result;
}

std::string Description::serialize() const {

	// std::stringstream ss;

	// for(auto c : m_markDown) {
	// 	if(c == '"')
	// 		ss << "\\\"";
	// 	else if(c == '\n')
	// 		ss << "\\n";
	// 	else if(c == '\r')
	// 		ss << "\\r";
	// 	else
	// 		ss << c;
	// }

	// return ss.str();

	return m_markDown;
}

void Description::deserialize(const std::string& s) {
	// std::string result;

	// auto it = s.begin();
	// while(it != s.end()) {
	// 	if(*it == '\\') {
	// 		++it;
	// 		if(it != s.end()) {
	// 			if(*it == 'n')
	// 				result += '\n';
	// 			else if(*it == 'r')
	// 				result += '\r';
	// 			else if(*it == '"')
	// 				result += '"';
	// 			else
	// 				result += "\\" + *it;
	// 		}
	// 	}
	// 	else
	// 		result += *it;

	// 	++it;
	// }

	// m_markDown = result;

	m_markDown = s;
}

}
