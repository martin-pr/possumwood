#pragma once

#include <set>
#include <string>

#include <boost/filesystem/path.hpp>

#include <dependency_graph/data_traits.h>

#include "possumwood_sdk/filepath.h"

#include "actions/io.h"
#include "actions/traits.h"

namespace possumwood {

class Filename {
  public:
	Filename(std::initializer_list<std::string> extensions = std::initializer_list<std::string>());

	const boost::filesystem::path filename(bool makeAbsolute = true) const;
	void setFilename(const boost::filesystem::path& filename);

	const std::set<std::string>& extensions() const;

	Filename(const Filename& fn);
	Filename& operator=(const Filename& fn);

	bool operator==(const Filename& fn) const;
	bool operator!=(const Filename& fn) const;

  private:
	possumwood::Filepath m_filename;
	std::set<std::string> m_extensions;
};

template <>
struct Traits<Filename> {
	static IO<Filename> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 0.5}};
	}
};

std::ostream& operator<<(std::ostream& out, const Filename& f);

}  // namespace possumwood
