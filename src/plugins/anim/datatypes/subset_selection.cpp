#include "subset_selection.h"

namespace anim {

SubsetSelection::Options::Options() : m_parent(NULL) {
}

SubsetSelection::Options::Options(SubsetSelection* parent) : m_parent(parent) {
}

SubsetSelection::Options::Options(const Options &o) : m_options(o.m_options), m_parent(NULL) {

}

SubsetSelection::Options& SubsetSelection::Options::operator =(const Options& o) {
	m_options = o.m_options;

	if(m_parent)
		m_parent->updateOptions();

	return *this;
}

void SubsetSelection::Options::add(const std::string& val) {
	m_options.insert(val);

	if(m_parent)
		m_parent->updateOptions();
}

void SubsetSelection::Options::clear() {
	m_options.clear();

	if(m_parent)
		m_parent->updateOptions();
}

SubsetSelection::Options::const_iterator SubsetSelection::Options::begin() const {
	return m_options.begin();
}

SubsetSelection::Options::const_iterator SubsetSelection::Options::end() const {
	return m_options.end();
}

SubsetSelection::Options::const_iterator SubsetSelection::Options::find(const std::string& val)
const {
	return m_options.find(val);
}

std::size_t SubsetSelection::Options::size() const {
	return m_options.size();
}

bool SubsetSelection::Options::empty() const {
	return m_options.empty();
}

bool SubsetSelection::Options::operator ==(const Options& o) const {
	return m_options == o.m_options;
}

bool SubsetSelection::Options::operator !=(const Options& o) const {
	return m_options != o.m_options;
}

///

SubsetSelection::const_iterator::const_iterator() {

}

SubsetSelection::const_iterator::const_iterator(Options::const_iterator optIt,
                                                Options::const_iterator optEnd,
                                                std::map<std::string, bool>::const_iterator selIt,
                                                std::map<std::string,
                                                         bool>::const_iterator selEnd) : m_optIt(
		optIt), m_optEnd(optEnd), m_selIt(selIt), m_selEnd(
		selEnd) {

}

void SubsetSelection::const_iterator::increment() {
	++m_optIt;
	if(m_optIt == m_optEnd)
		m_selIt = m_selEnd;

	else
		while(m_selIt->first < *m_optIt) {
			++m_selIt;
			assert(m_selIt != m_selEnd);
		}
}

bool SubsetSelection::const_iterator::equal(const const_iterator& ci) const {
	return m_optIt == ci.m_optIt;
}

const std::pair<const std::string, bool>& SubsetSelection::const_iterator::dereference() const {
	return *m_selIt;
}

///

SubsetSelection::SubsetSelection() : m_options(this) {
}

SubsetSelection::SubsetSelection(const SubsetSelection& ss) : m_options(this), m_selection(
		ss.m_selection) {
	// use assignment operator - does not change the "parent"
	m_options = ss.m_options;
}

SubsetSelection& SubsetSelection::operator =(const SubsetSelection& s) {
	m_selection = s.m_selection;
	m_options = s.m_options;

	return *this;
}

SubsetSelection::Options& SubsetSelection::options() {
	return m_options;
}

const SubsetSelection::Options& SubsetSelection::options() const {
	return m_options;
}

SubsetSelection::const_iterator SubsetSelection::begin() const {
	if(m_options.empty())
		return end();

	return const_iterator(m_options.begin(), m_options.end(),
	                      m_selection.lower_bound(*m_options.begin()),
	                      m_selection.end());
}

SubsetSelection::const_iterator SubsetSelection::end() const {
	return const_iterator(m_options.end(), m_options.end(), m_selection.end(), m_selection.end());
}

SubsetSelection::const_iterator SubsetSelection::find(const std::string& val) const {
	auto it1 = m_options.find(val);
	if(it1 == m_options.end())
		return end();

	auto it2 = m_selection.find(val);
	assert(it2 != m_selection.end());

	return const_iterator(it1, m_options.end(), it2, m_selection.end());
}

void SubsetSelection::select(const std::string& val) {
	m_selection[val] = true;
}

void SubsetSelection::deselect(const std::string& val) {
	m_selection[val] = false;
}

void SubsetSelection::clear() {
	m_selection.clear();
}

bool SubsetSelection::operator ==(const SubsetSelection& ss) const {
	return m_selection == ss.m_selection && m_options == ss.m_options;
}

bool SubsetSelection::operator !=(const SubsetSelection& ss) const {
	return m_selection != ss.m_selection || m_options != ss.m_options;
}

void SubsetSelection::updateOptions() {
	// make sure there is an item for all options (default to false)
	for(auto& o : m_options) {
		auto it = m_selection.find(o);
		if(it == m_selection.end())
			m_selection[o] = false;
	}
}

}

namespace possumwood {

namespace {

void toJson(::dependency_graph::io::json& json, const anim::SubsetSelection& value) {
	for(auto& i : value)
		if(i.second)
			json.push_back(i.first);
}

void fromJson(const ::dependency_graph::io::json& json, anim::SubsetSelection& value) {
	for(auto& i : json)
		value.select(i.get<std::string>());
}

}

possumwood::IO<anim::SubsetSelection> Traits<anim::SubsetSelection>::io(
	&toJson, &fromJson);

};
