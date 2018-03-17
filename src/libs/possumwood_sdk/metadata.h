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

class Metadata : public dependency_graph::Metadata {
	public:
		Metadata(const std::string& nodeType);
		virtual ~Metadata();


		/// compute method of this node


		/// drawable for this node type - sets the drawable to be of the type
		///   passed as template argument
		template<typename DRAWABLE>
		void setDrawable();

		/// drawable for this node type, as a simple functor
		void setDrawable(std::function<dependency_graph::State(const dependency_graph::Values&, const possumwood::Drawable::ViewportState&)> fn);

		/// creates a new drawable instance for given value set
		std::unique_ptr<Drawable> createDrawable(dependency_graph::Values&& values) const;


		/// set the type used as editor for this node type
		template<typename EDITOR>
		void setEditor();

		/// returns true if this node type has an editor set
		bool hasEditor() const;
		/// create an editor for a node instance
		std::unique_ptr<Editor> createEditor(dependency_graph::NodeBase& node) const;


		/// colour of an attribute, based on its index (derived from Traits instances)
		const std::array<float, 3>& colour(unsigned attrId) const;

	protected:
		virtual void doAddAttribute(dependency_graph::Attr& a) override;

	private:
		std::function<std::unique_ptr<Drawable>(dependency_graph::Values&&)> m_drawableFactory;
		std::function<std::unique_ptr<Editor>(dependency_graph::NodeBase&)> m_editorFactory;

		std::vector<std::array<float, 3>> m_colours;
};

}
