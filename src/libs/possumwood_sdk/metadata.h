#pragma once

#include <set>
#include <vector>
#include <array>

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <dependency_graph/metadata.inl>

#include "drawable.h"
#include "editor.h"

namespace possumwood {

class ActionsAccess;

class Metadata : public dependency_graph::Metadata {
	public:
		Metadata(const std::string& nodeType);
		virtual ~Metadata();

		using dependency_graph::Metadata::addAttribute;

		/// registers an input attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(dependency_graph::InAttr<T>& in, const std::string& name, const T& defaultValue = T(), unsigned flags = 0);

		/// registers an output attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(dependency_graph::OutAttr<T>& out, const std::string& name, const T& defaultValue = T(), unsigned flags = 0);

		using dependency_graph::Metadata::addInfluence;
		using dependency_graph::Metadata::setCompute;

		/// drawable for this node type - sets the drawable to be of the type
		///   passed as template argument
		template<typename DRAWABLE>
		void setDrawable();

		/// drawable for this node type, as a simple functor
		void setDrawable(std::function<dependency_graph::State(const dependency_graph::Values&, const possumwood::ViewportState&)> fn);

		/// creates a new drawable instance for given value set
		std::unique_ptr<Drawable> createDrawable(dependency_graph::Values&& values) const;


		/// set the type used as editor for this node type
		template<typename EDITOR>
		void setEditor();

		/// set the editor factory directly (allows for passing additional parameters to the constructor)
		void setEditorFactory(const std::function<std::unique_ptr<Editor>()>& editorFactory);

		/// returns true if this node type has an editor set
		bool hasEditor() const;
		/// create an editor for a node instance
		std::unique_ptr<Editor> createEditor(dependency_graph::NodeBase& node) const;

		virtual std::unique_ptr<dependency_graph::NodeBase> createNode(const std::string& name, dependency_graph::Network& parent, const dependency_graph::UniqueId& id = dependency_graph::UniqueId()) const override;

		/// draw a node instance, calling drawable's doDraw() function, if present
		static boost::optional<Drawable&> getDrawable(const dependency_graph::NodeBase& node);

	private:
		std::function<std::unique_ptr<Drawable>(dependency_graph::Values&&)> m_drawableFactory;
		std::function<std::unique_ptr<Editor>()> m_editorFactory;
};

}
