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

Description::Description(const std::string& md) : m_markDown(md) {
}

void Description::clear() {
	m_markDown = "(empty description)";
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

		if(starts_with(line, "# "))
			line = "<h1>" + line.substr(2) + "</h1>\n";
		else if(starts_with(line, "## "))
			line = "<h2>" + line.substr(3) + "</h2>\n";
		else if(starts_with(line, "### "))
			line = "<h3>" + line.substr(4) + "</h3>\n";
		else if(starts_with(line, "#### "))
			line = "<h4>" + line.substr(5) + "</h4>\n";
		else if(!line.empty())
			line = "<p>" + line + "</p>\n";

		{
			std::string link_text, link;
			int state = 0;
			std::size_t start = 0;

			for(std::size_t i=0; i<line.length(); ++i) {
				switch(state) {
					case 0:
						if(line[i] == '[') {
							state = 1;
							start = i;
						}
						break;

					case 1:
						if(line[i] == ']')
							state = 2;
						else
							link_text += line[i];
						break;

					case 2:
						if(line[i] == '(')
							state = 3;
						else
							state = 0;
						break;

					case 3:
						if(line[i] == ')')
							state = 4;
						else
							link += line[i];
						break;

					case 4:
						line = line.substr(0, start) + "<a href=\"" + link + "\">" + link_text + "</a>" + line.substr(i);
						i = 0;
						state = 0;
						break;
				};
			}
		}

		{
			int state = 0;
			for(std::size_t i=0; i<line.length(); ++i) {
				switch(state) {
					case 0:
						if(line[i] == '`') {
							line = line.substr(0, i) + "<code>" + line.substr(i+1);
							state = 1;
						}
						break;
					case 1:
						if(line[i] == '`') {
							line = line.substr(0, i) + "</code>" + line.substr(i+1);
							state = 0;
						}
						break;
				}
			}
		}

		result += line;
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
