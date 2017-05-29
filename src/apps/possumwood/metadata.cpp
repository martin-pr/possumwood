#include "metadata.h"

std::set<Metadata*, Metadata::Comparator> Metadata::s_instances;

Metadata::Metadata(const std::string& nodeType) : dependency_graph::Metadata(nodeType) {
	s_instances.insert(this);
}

Metadata::~Metadata() {
	auto it = s_instances.find(this);
	assert(it != s_instances.end());

	s_instances.erase(it);
}

void Metadata::setDraw(std::function<void(const dependency_graph::Values&)> drawFunctor) {
	m_drawFunctor = drawFunctor;
}

void Metadata::draw(const dependency_graph::Values& vals) const {
	if(m_drawFunctor)
		m_drawFunctor(vals);
}

boost::iterator_range<Metadata::const_iterator> Metadata::instances() {
	return boost::make_iterator_range(
		s_instances.begin(),
		s_instances.end()
	);
}

const Metadata& Metadata::instance(const std::string& nodeType) {
	/// TODO: improve efficiency of search
	for(auto& i : s_instances)
		if(i->type() == nodeType)
			return *i;

	throw(std::runtime_error("node type " + nodeType + " is not registered"));
}

