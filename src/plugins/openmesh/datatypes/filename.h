#pragma once

#include <string>
#include <set>

#include <boost/filesystem/path.hpp>

#include <dependency_graph/data_traits.h>

class Filename {
	public:
		Filename(std::initializer_list<std::string> extensions = std::initializer_list<std::string>());

		const boost::filesystem::path filename(bool makeAbsolute = true) const;
		void setFilename(const boost::filesystem::path& filename);

		const std::set<std::string>& extensions() const;


	private:
		boost::filesystem::path m_filename;
		std::set<std::string> m_extensions;
};

namespace dependency_graph {
	// traits specialisation for filename, to assign only the filename value,
	//   not the extensions
	template<>
	struct DataTraits<Filename> {
		static void assignValue(Filename& dest, const Filename& src) {
			dest.setFilename(src.filename());
		}

		static bool isEqual(const Filename& v1, const Filename& v2) {
			return v1.filename() == v2.filename();
		}

		static bool isNotEqual(const Filename& v1, const Filename& v2) {
			return v1.filename() != v2.filename();
		}
	};
}
