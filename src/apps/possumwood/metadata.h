#pragma once

#include <set>

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <dependency_graph/metadata.inl>

class Metadata : public dependency_graph::Metadata {
	public:
		Metadata(const std::string& nodeType);
		virtual ~Metadata();

		/// compute method of this node
		void setDraw(std::function<void(const dependency_graph::Values&)> drawFunctor);

		/// executes the draw method
		void draw(const dependency_graph::Values& vals) const;

	private:
		std::function<void(const dependency_graph::Values&)> m_drawFunctor;
};
