#pragma once

#include <set>

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <dependency_graph/metadata.inl>

#include "drawable.h"

namespace possumwood {

class Metadata : public dependency_graph::Metadata {
	public:
		Metadata(const std::string& nodeType);
		virtual ~Metadata();

		/// compute method of this node
		void setDrawableFactory(std::function<std::unique_ptr<Drawable>(dependency_graph::Values&&)> drawableFactory);

		/// creates a new drawable instance for given value set
		std::unique_ptr<Drawable> createDrawable(dependency_graph::Values&& values) const;

	private:
		std::function<std::unique_ptr<Drawable>(dependency_graph::Values&&)> m_drawableFactory;
};

}
