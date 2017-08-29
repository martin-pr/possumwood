#include "metadata.h"

#include <cassert>

namespace dependency_graph {

std::set<Metadata*, Metadata::Comparator>& Metadata::instanceSet() {
	static std::unique_ptr<std::set<Metadata*, Metadata::Comparator>> s_instances;
	if(s_instances == nullptr)
		s_instances = std::unique_ptr<std::set<Metadata*, Metadata::Comparator>>(new std::set<Metadata*, Metadata::Comparator>());

	return *s_instances;
}

Metadata::Metadata(const std::string& nodeType) : m_type(nodeType) {
	instanceSet().insert(this);
}

Metadata::~Metadata() {
	auto it = instanceSet().find(this);
	if(it != instanceSet().end())
		instanceSet().erase(it);
}

bool Metadata::isValid() const {
	return !m_attrs.empty() && m_compute;
}

const std::string& Metadata::type() const {
	return m_type;
}

void Metadata::setCompute(std::function<State(Values&)> compute) {
	m_compute = compute;
}

size_t Metadata::attributeCount() const {
	return m_attrs.size();
}

Attr& Metadata::attr(size_t index) {
	assert(index < m_attrs.size());
	return *m_attrs[index];
}

const Attr& Metadata::attr(size_t index) const {
	assert(index < m_attrs.size());
	return *m_attrs[index];
}

std::vector<std::reference_wrapper<const Attr>> Metadata::influences(size_t index) const {
	std::vector<std::reference_wrapper<const Attr>> result;

	auto i1 = m_influences.left.lower_bound(index);
	auto i2 = m_influences.left.upper_bound(index);
	for(auto i = i1; i != i2; ++i)
		result.push_back(attr(i->second));

	return result;
}

std::vector<std::reference_wrapper<const Attr>> Metadata::influencedBy(size_t index) const {
	std::vector<std::reference_wrapper<const Attr>> result;

	auto i1 = m_influences.right.lower_bound(index);
	auto i2 = m_influences.right.upper_bound(index);
	for(auto i = i1; i != i2; ++i)
		result.push_back(attr(i->second));

	return result;
}

boost::iterator_range<Metadata::const_iterator> Metadata::instances() {
	return boost::make_iterator_range(
		instanceSet().begin(),
		instanceSet().end()
	);
}

const Metadata& Metadata::instance(const std::string& nodeType) {
	/// TODO: improve efficiency of search
	for(auto& i : instanceSet())
		if(i->type() == nodeType)
			return *i;

	throw(std::runtime_error("node type " + nodeType + " is not registered"));
}

}
