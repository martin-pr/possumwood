#include "metadata.h"

Metadata::Metadata(const std::string& nodeType) : dependency_graph::Metadata(nodeType) {
}

Metadata::~Metadata() {
}

void Metadata::setDraw(std::function<void(const dependency_graph::Values&)> drawFunctor) {
	m_drawFunctor = drawFunctor;
}

void Metadata::draw(const dependency_graph::Values& vals) const {
	if(m_drawFunctor)
		m_drawFunctor(vals);
}
