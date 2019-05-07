#include "metadata.h"

#include "actions/traits.h"

namespace possumwood {

namespace {

class PossumwoodNode : public dependency_graph::Node {
	public:
		PossumwoodNode(const std::string& name, const dependency_graph::UniqueId& id, const dependency_graph::MetadataHandle& def, dependency_graph::Network* parent) : dependency_graph::Node(name, id, def, parent) {
			const possumwood::Metadata& meta = dynamic_cast<const possumwood::Metadata&>(def.metadata());

			m_drawable = meta.createDrawable(dependency_graph::Values(*this));
		}

		boost::optional<Drawable&> drawable() const {
			if(m_drawable)
				return *m_drawable;
			return boost::optional<Drawable&>();
		}

	private:
		std::unique_ptr<Drawable> m_drawable;
};

}

Metadata::Metadata(const std::string& nodeType) : dependency_graph::Metadata(nodeType) {
}

Metadata::~Metadata() {
}

void Metadata::addAttribute(dependency_graph::InAttr<void>& in, const std::string& name, AttrFlags flags) {
	dependency_graph::Metadata::addAttribute(in, name, static_cast<unsigned>(flags));
}

void Metadata::addAttribute(dependency_graph::OutAttr<void>& in, const std::string& name, AttrFlags flags) {
	dependency_graph::Metadata::addAttribute(in, name, static_cast<unsigned>(flags));
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

void Metadata::setEditorFactory(const std::function<std::unique_ptr<Editor>()>& editorFactory) {
	m_editorFactory = editorFactory;
}

std::unique_ptr<Editor> Metadata::createEditor(dependency_graph::NodeBase& node) const {
	assert(hasEditor());

	// create a new editor instance
	std::unique_ptr<Editor> editor = m_editorFactory();
	// set the edited node reference
	editor->setNodeReference(node);

	return editor;
}

std::unique_ptr<dependency_graph::NodeBase> Metadata::createNode(const std::string& name, dependency_graph::Network& parent, const dependency_graph::UniqueId& id) const {
	return std::unique_ptr<dependency_graph::NodeBase>(new PossumwoodNode(name, id, *this, &parent));
};

boost::optional<Drawable&> Metadata::getDrawable(const dependency_graph::NodeBase& node) {
	const PossumwoodNode* instance = dynamic_cast<const PossumwoodNode*>(&node);
	if(instance)
		return instance->drawable();
	return boost::optional<Drawable&>();
}

////////////////

namespace {

struct Factory : public dependency_graph::MetadataFactory {
	virtual std::unique_ptr<dependency_graph::Metadata> instantiate(const std::string& type) {
		// special handling for "network" type - we need to create new metadata for each network
		//   with different inputs / outputs. But, at least for now, a network is not reimplemented
		//   in Possumwood and should go straight from dependency_graph library.
		if(type == "network")
			return std::unique_ptr<dependency_graph::Metadata>(new dependency_graph::Metadata(type));
		else
			return std::unique_ptr<dependency_graph::Metadata>(new Metadata(type));
	}
};

static Factory s_factoryInstance;

}

}
