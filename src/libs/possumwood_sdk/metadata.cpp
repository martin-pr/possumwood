#include "metadata.h"

namespace possumwood {

Metadata::Metadata(const std::string& nodeType) : dependency_graph::Metadata(nodeType) {
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

const std::array<float, 3>& Metadata::colour(unsigned attrId) const {
	assert(attrId < m_colours.size());
	return m_colours[attrId];
}

bool Metadata::hasEditor() const {
	return m_editorFactory.operator bool();
}

std::unique_ptr<Editor> Metadata::createEditor(dependency_graph::NodeBase& node) const {
	assert(hasEditor());

	return m_editorFactory(node);
}

void Metadata::doAddAttribute(dependency_graph::Attr& a) {
	dependency_graph::Metadata::doAddAttribute(a);

	// just a silly workaround when the base addAttribute() gets called instead
	//   of the derived class version
	while(m_colours.size() < attributeCount())
		m_colours.push_back(std::array<float, 3>{{1,0,1}});
}

}
