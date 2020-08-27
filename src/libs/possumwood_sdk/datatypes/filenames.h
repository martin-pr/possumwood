#pragma once

#include <set>
#include <string>

#include <boost/filesystem/path.hpp>

#include <actions/filepath.h>
#include <dependency_graph/data_traits.h>

#include "actions/io.h"
#include "actions/traits.h"

namespace possumwood {

class Filenames {
  public:
	Filenames(std::initializer_list<std::string> extensions = std::initializer_list<std::string>());

	const std::vector<possumwood::Filepath>& filenames() const;

	bool empty() const;
	void addFilename(const possumwood::Filepath& filename);
	void clear();

	const std::set<std::string>& extensions() const;

	Filenames(const Filenames& fn);
	Filenames& operator=(const Filenames& fn);

	bool operator==(const Filenames& fn) const;
	bool operator!=(const Filenames& fn) const;

  private:
	std::vector<possumwood::Filepath> m_filenames;
	std::set<std::string> m_extensions;
};

template <>
struct Traits<Filenames> {
	static IO<Filenames> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 1, 0.5}};
	}
};

std::ostream& operator<<(std::ostream& out, const Filenames& f);

}  // namespace possumwood
