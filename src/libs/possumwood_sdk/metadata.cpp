#include "metadata.h"

namespace possumwood {

Metadata::Metadata(const std::string& nodeType) : m_meta(nodeType) {
	m_meta.setBlindData<Metadata*>(this);
}

Metadata::~Metadata() {
}

void Metadata::setDrawable(std::function<dependency_graph::State(const dependency_graph::Values&, const possumwood::Drawable::ViewportState&)> fn) {
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

const std::array<float, 3>& Metadata::colour(unsigned attrId) const {
	assert(attrId < m_colours.size());
	return m_colours[attrId];
}

bool Metadata::hasEditor() const {
	return m_editorFactory.operator bool();
}

std::unique_ptr<Editor> Metadata::createEditor(dependency_graph::NodeBase& node) {
	assert(hasEditor());

	return m_editorFactory(node);
}

}
