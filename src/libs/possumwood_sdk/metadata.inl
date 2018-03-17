#pragma once

#include "metadata.h"
#include "traits.h"
#include "editor.h"

namespace possumwood {

template<typename DRAWABLE>
void Metadata::setDrawable() {
	m_drawableFactory = [](dependency_graph::Values&& vals) {
		return std::unique_ptr<possumwood::Drawable>(
			new DRAWABLE(std::move(vals)));
	};
}

template<typename EDITOR>
void Metadata::setEditor() {
	m_editorFactory = [](dependency_graph::NodeBase& node) {
		std::unique_ptr<possumwood::Editor> result(new EDITOR());

		result->setNodeReference(node);

		return result;
	};
}







}
