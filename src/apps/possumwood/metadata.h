#pragma once

#include <set>

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include <dependency_graph/metadata.inl>

class Metadata : public dependency_graph::Metadata {
	private:
		struct Comparator {
			bool operator()(const Metadata* m1, const Metadata* m2) const {
				return m1->type() < m2->type();
			}
		};

	public:
		typedef boost::indirect_iterator<std::set<Metadata*, Comparator>::const_iterator> const_iterator;

		/// returns all existing instances. Used for enumerating all instantiable nodes.
		static boost::iterator_range<const_iterator> instances();
		/// returns a single metadata instance (throws if not found)
		static const Metadata& instance(const std::string& nodeType);

		Metadata(const std::string& nodeType);
		virtual ~Metadata();

		/// compute method of this node
		void setDraw(std::function<void(const dependency_graph::Values&)> drawFunctor);

		/// executes the draw method
		void draw(const dependency_graph::Values& vals) const;

	private:
		std::function<void(const dependency_graph::Values&)> m_drawFunctor;

		static std::set<Metadata*, Metadata::Comparator> s_instances;
};
