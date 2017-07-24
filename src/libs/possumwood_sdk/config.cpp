#include "config.inl"

#include <cassert>
#include <algorithm>

namespace possumwood {

Config::Item::Item(const Item& i) : m_name(i.name()), m_group(i.group()), m_description(i.description()), m_flags(i.flags()), m_value(i.m_value), m_defaultValue(i.m_defaultValue) {
}

Config::Item& Config::Item::operator = (const Item& i) {
	m_name = i.m_name;
	m_group = i.m_group;
	m_description = i.m_description;
	m_flags = i.m_flags;

	m_value = i.m_value;
	m_defaultValue = i.m_defaultValue;

	m_onChanged(*this);

	return *this;
}

const std::string& Config::Item::name() const {
	return m_name;
}

const std::string& Config::Item::group() const {
	return m_group;
}

const std::string Config::Item::type() const {
	if(is<std::string>())
		return "std::string";
	else if(is<int>())
		return "int";
	else if(is<float>())
		return "float";
	else
		assert(false);
}

const std::string& Config::Item::description() const {
	return m_description;
}

const Config::Item::Flags& Config::Item::flags() const {
	return m_flags;
}

boost::signals2::connection Config::Item::onChanged(std::function<void(Item&)> callback) {
	return m_onChanged.connect(callback);
}

///////////

Config::~Config() {

}

namespace {
	struct Compare {
		bool operator() (const Config::Item& i, const std::string& name) const {
			return i.name() < name;
		}

		bool operator() (const std::string& name, const Config::Item& i) const {
			return name < i.name();
		}
	};
}

Config::Item& Config::operator[](const std::string& name) {
	std::vector<Item>::iterator it = std::lower_bound(m_items.begin(), m_items.end(), name, Compare());
	assert(it != m_items.end());
	assert(it->name() == name);

	return *it;
}

const Config::Item& Config::operator[](const std::string& name) const {
	std::vector<Item>::const_iterator it = std::lower_bound(m_items.begin(), m_items.end(), name, Compare());
	assert(it != m_items.end());
	assert(it->name() == name);

	return *it;
}

Config::const_iterator Config::begin() const {
	return m_items.begin();
}

Config::const_iterator Config::end() const {
	return m_items.end();
}

Config::iterator Config::begin() {
	return m_items.begin();
}

Config::iterator Config::end() {
	return m_items.end();
}

void Config::addItem(const Item& i) {
	auto it = std::upper_bound(m_items.begin(), m_items.end(), i.name(), Compare());

	m_items.insert(it, i);
}

void Config::reset() {
	for(auto& i : m_items) {
		i.m_value = i.m_defaultValue;
		i.m_onChanged(i);
	}
}

}
