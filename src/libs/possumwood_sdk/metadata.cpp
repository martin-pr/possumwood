#include "metadata.h"

namespace possumwood {

Metadata::Metadata(const std::string& nodeType) : dependency_graph::Metadata(nodeType) {
}

Metadata::~Metadata() {
}

void Metadata::setDrawableFactory(std::function<std::unique_ptr<Drawable>(dependency_graph::Values&&)> drawableFactory) {
	m_drawableFactory = drawableFactory;
}

std::unique_ptr<Drawable> Metadata::createDrawable(dependency_graph::Values&& values) const {
	if(m_drawableFactory)
		return m_drawableFactory(std::move(values));

	return std::unique_ptr<Drawable>();
}

}
