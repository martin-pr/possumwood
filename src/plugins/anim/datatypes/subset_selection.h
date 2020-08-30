#pragma once

#include <actions/traits.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/noncopyable.hpp>
#include <set>

namespace anim {

/// a simple class allowing to pick a number of string items (represent a set of checkboxes UI)
class SubsetSelection {
  public:
	SubsetSelection();

	SubsetSelection(const SubsetSelection& ss);
	SubsetSelection& operator=(const SubsetSelection& s);

	/// a list of selectable options
	class Options {
	  public:
		Options();
		Options(const Options& o);
		Options& operator=(const Options& o);

		void add(const std::string& val);
		void clear();

		typedef std::set<std::string>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;
		const_iterator find(const std::string& val) const;

		std::size_t size() const;
		bool empty() const;

		bool operator==(const Options& o) const;
		bool operator!=(const Options& o) const;

	  private:
		Options(SubsetSelection* parent);

		std::set<std::string> m_options;
		SubsetSelection* m_parent;

		friend class SubsetSelection;
	};

	Options& options();
	const Options& options() const;

	class const_iterator
	    : public boost::iterator_facade<const_iterator, std::pair<const std::string, bool>,
	                                    boost::forward_traversal_tag, const std::pair<const std::string, bool>&> {
	  public:
		const_iterator();

	  private:
		const_iterator(Options::const_iterator m_optIt, Options::const_iterator m_optEnd,
		               std::map<std::string, bool>::const_iterator m_selIt,
		               std::map<std::string, bool>::const_iterator m_selEnd);

		Options::const_iterator m_optIt, m_optEnd;
		std::map<std::string, bool>::const_iterator m_selIt, m_selEnd;

		void increment();
		bool equal(const const_iterator& ci) const;
		const std::pair<const std::string, bool>& dereference() const;

		friend class boost::iterator_core_access;
		friend class SubsetSelection;
	};

	const_iterator begin() const;
	const_iterator end() const;
	const_iterator find(const std::string& val) const;

	void select(const std::string& val);
	void deselect(const std::string& val);
	void clear();

	bool operator==(const SubsetSelection& ss) const;
	bool operator!=(const SubsetSelection& ss) const;

  private:
	Options m_options;
	std::map<std::string, bool> m_selection;

	void updateOptions();

	friend class Options;
};

std::ostream& operator<<(std::ostream& out, const SubsetSelection& ss);

}  // namespace anim

namespace possumwood {

template <>
struct Traits<anim::SubsetSelection> {
	static possumwood::IO<anim::SubsetSelection> io;

	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1.0, 1.0, 1.0}};
	}
};

}  // namespace possumwood
