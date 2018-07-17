#include "metadata.h"

#include "actions/traits.h"

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

void Metadata::addAttribute(dependency_graph::InAttr<void>& in, const std::string& name) {
	m_colours.push_back(Traits<void>::colour());

	dependency_graph::Metadata::addAttribute(in, name);
}

void Metadata::addAttribute(dependency_graph::OutAttr<void>& out, const std::string& name) {
	m_colours.push_back(Traits<void>::colour());

	dependency_graph::Metadata::addAttribute(out, name);
}

}

namespace dependency_graph {

std::unique_ptr<Metadata> instantiateMetadata(const std::string& type) {
	return std::unique_ptr<Metadata>(new possumwood::Metadata(type));
}

}
