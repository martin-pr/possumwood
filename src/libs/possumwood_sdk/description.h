#pragma once

#include <string>

namespace possumwood {

/// A description of a setup, using a subset of markdown
class Description {
	public:
		void clear();

		void setMarkdown(const std::string& md);
		const std::string& markdown() const;

		std::string html() const;

		std::string serialize() const;
		void deserialize(const std::string& s);

	private:
		std::string m_markDown;
};

}
