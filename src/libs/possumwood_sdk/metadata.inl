#pragma once

#include "metadata.h"

namespace possumwood {

template<typename DRAWABLE>
void Metadata::setDrawable() {
	m_drawableFactory = [](dependency_graph::Values&& vals) {
		return std::unique_ptr<possumwood::Drawable>(
			new DRAWABLE(std::move(vals)));
	};
}

template<typename T>
void Metadata::addAttribute(dependency_graph::InAttr<T>& in, const std::string& name, const T& defaultValue) {
	m_meta.addAttribute(in, name, defaultValue);
}

template<typename T>
void Metadata::addAttribute(dependency_graph::OutAttr<T>& out, const std::string& name, const T& defaultValue) {
	m_meta.addAttribute(out, name, defaultValue);
}

template<typename T, typename U>
void Metadata::addInfluence(const dependency_graph::InAttr<T>& in, const dependency_graph::OutAttr<U>& out) {
	m_meta.addInfluence(in, out);
}

}
