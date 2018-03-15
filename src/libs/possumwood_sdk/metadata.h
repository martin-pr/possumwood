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

class Metadata : public boost::noncopyable {
	public:
		Metadata(const std::string& nodeType);
		virtual ~Metadata();

		/// registers an input attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(dependency_graph::InAttr<T>& in, const std::string& name, const T& defaultValue = T());

		/// registers an output attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(dependency_graph::OutAttr<T>& out, const std::string& name, const T& defaultValue = T());

		/// adds attribute influence - inputs required to compute outputs
		template<typename T, typename U>
		void addInfluence(const dependency_graph::InAttr<T>& in, const dependency_graph::OutAttr<U>& out);

		/// compute method of this node
		void setCompute(std::function<dependency_graph::State(dependency_graph::Values&)> compute);


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
		std::unique_ptr<Editor> createEditor(dependency_graph::NodeBase& node);


		/// colour of an attribute, based on its index (derived from Traits instances)
		const std::array<float, 3>& colour(unsigned attrId) const;

	private:
		dependency_graph::Metadata m_meta;

		std::function<std::unique_ptr<Drawable>(dependency_graph::Values&&)> m_drawableFactory;
		std::function<std::unique_ptr<Editor>(dependency_graph::NodeBase&)> m_editorFactory;

		std::vector<std::array<float, 3>> m_colours;
};

}
