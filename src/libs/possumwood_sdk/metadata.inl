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

template<typename T>
void Metadata::addAttribute(dependency_graph::InAttr<T>& in, const std::string& name, const T& defaultValue) {
	dependency_graph::Metadata::addAttribute(in, name, defaultValue);

	m_colours.push_back(Traits<T>::colour());
}

template<typename T>
void Metadata::addAttribute(dependency_graph::OutAttr<T>& out, const std::string& name, const T& defaultValue) {
	dependency_graph::Metadata::addAttribute(out, name, defaultValue);

	m_colours.push_back(Traits<T>::colour());
}

}
