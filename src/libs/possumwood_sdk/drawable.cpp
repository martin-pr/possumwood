#include "drawable.h"

namespace possumwood {

boost::signals2::signal<void()> Drawable::s_refresh;

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

boost::signals2::connection Drawable::onRefreshQueued(std::function<void()> fn) {
	return s_refresh.connect(fn);
}

void Drawable::refresh() {
	s_refresh();
}

//////////

DrawableFunctor::DrawableFunctor(dependency_graph::Values&& vals, std::function<dependency_graph::State(const dependency_graph::Values&)> draw) : Drawable(std::move(vals)), m_draw(draw) {
}

dependency_graph::State DrawableFunctor::draw() {
	if(m_draw)
		return m_draw(values());

	return dependency_graph::State();
}

}
