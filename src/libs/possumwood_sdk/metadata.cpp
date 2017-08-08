#include "metadata.h"

namespace possumwood {

Metadata::Metadata(const std::string& nodeType) : m_meta(nodeType) {
	m_meta.setBlindData<Metadata*>(this);
}

Metadata::~Metadata() {
}

void Metadata::setDrawable(std::function<void(const dependency_graph::Values&)> fn) {
	m_drawableFactory = [fn](dependency_graph::Values&& vals) {
		return std::unique_ptr<possumwood::Drawable>(
			new possumwood::DrawableFunctor(std::move(vals), fn));
	};
}

std::unique_ptr<Drawable> Metadata::createDrawable(dependency_graph::Values&& values) const {
	if(m_drawableFactory)
		return m_drawableFactory(std::move(values));

	return std::unique_ptr<Drawable>();
}

void Metadata::setCompute(std::function<dependency_graph::State(dependency_graph::Values&)> compute) {
	m_meta.setCompute(compute);
}

}
