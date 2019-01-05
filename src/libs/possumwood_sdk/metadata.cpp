#include "metadata.h"

#include "actions/traits.h"

namespace possumwood {

Metadata::Metadata(const std::string& nodeType) : dependency_graph::Metadata(nodeType) {
}

Metadata::~Metadata() {
}

void Metadata::setDrawable(std::function<dependency_graph::State(const dependency_graph::Values&, const possumwood::ViewportState&)> fn) {
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

bool Metadata::hasEditor() const {
	return m_editorFactory.operator bool();
}

std::unique_ptr<Editor> Metadata::createEditor(dependency_graph::NodeBase& node) const {
	assert(hasEditor());

	return m_editorFactory(node);
}

////////////////

namespace {

struct Factory : public dependency_graph::MetadataFactory {
	virtual std::unique_ptr<dependency_graph::Metadata> instantiate(const std::string& type) {
		return std::unique_ptr<dependency_graph::Metadata>(new Metadata(type));
	}
};

static Factory s_factoryInstance;

}

}
