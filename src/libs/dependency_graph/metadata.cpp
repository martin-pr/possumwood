#include "metadata.h"

#include <cassert>

#include <boost/iterator/indirect_iterator.hpp>

#include "attr.h"

namespace dependency_graph {

class Metadata::Register {
  public:
	~Register() {
		while(!instances.empty())
			(*instances.begin())->doUnregister();
	}

	void add(Metadata* m) {
		instances.insert(m);
	}

	void remove(Metadata* m) {
		auto it = instances.find(m);
		assert(it != instances.end());

		instances.erase(it);
	}

	typedef boost::indirect_iterator<std::set<Metadata*, Metadata::Comparator>::iterator> iterator;

	iterator begin() {
		return boost::make_indirect_iterator(instances.begin());
	}

	iterator end() {
		return boost::make_indirect_iterator(instances.end());
	}

  private:
	std::set<Metadata*, Metadata::Comparator> instances;
};

Metadata::Register& Metadata::instanceSet() {
	static std::unique_ptr<Register> s_instances;
	if(s_instances == nullptr)
		s_instances = std::unique_ptr<Register>(new Register());

	return *s_instances;
}

void Metadata::doRegister() {
	if(!m_registered) {
		instanceSet().add(this);
		m_registered = true;
	}
}

void Metadata::doUnregister() {
	if(m_registered) {
		instanceSet().remove(this);
		m_registered = false;
	}
}

Metadata::Metadata(const std::string& nodeType) : m_type(nodeType), m_registered(false) {
	doRegister();
}

Metadata::~Metadata() {
	doUnregister();
}

bool Metadata::isValid() const {
	return !m_attrs.empty() && m_compute && m_registered;
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
	return m_attrs[index];
}

const Attr& Metadata::attr(size_t index) const {
	assert(index < m_attrs.size());
	return m_attrs[index];
}

void Metadata::doAddAttribute(Attr& attr) {
	m_attrs.push_back(attr);
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
	return boost::make_iterator_range(instanceSet().begin(), instanceSet().end());
}

const Metadata& Metadata::instance(const std::string& nodeType) {
	/// TODO: improve efficiency of search
	for(auto& i : instanceSet())
		if(i.type() == nodeType)
			return i;

	throw(std::runtime_error("node type " + nodeType + " is not registered"));
}

}
