#pragma once

#include <string>
#include <set>

#include <boost/filesystem/path.hpp>

#include <dependency_graph/data_traits.h>

#include "../io.h"
#include "../traits.h"

namespace possumwood {

class Filename {
	public:
		Filename(std::initializer_list<std::string> extensions = std::initializer_list<std::string>());

		const boost::filesystem::path filename(bool makeAbsolute = true) const;
		void setFilename(const boost::filesystem::path& filename);

		const std::set<std::string>& extensions() const;

		Filename& operator = (const Filename& fn);

		bool operator == (const Filename& fn) const;
		bool operator != (const Filename& fn) const;

	private:
		boost::filesystem::path m_filename;
		std::set<std::string> m_extensions;
};

template<>
struct Traits<Filename> {
	static IO<Filename> io;
};

}
