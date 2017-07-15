#include "drawable.h"

namespace possumwood {

Drawable::Drawable(dependency_graph::Values&& vals) : m_vals(std::move(vals)) {
}

Drawable::~Drawable() {
}

dependency_graph::Values& Drawable::values() {
	return m_vals;
}

const dependency_graph::Values& Drawable::values() const {
	return m_vals;
}

DrawableFunctor::DrawableFunctor(dependency_graph::Values&& vals, std::function<void(const dependency_graph::Values&)> draw) : Drawable(std::move(vals)), m_draw(draw) {
}

void DrawableFunctor::draw() {
	if(m_draw)
		m_draw(values());
}

}
