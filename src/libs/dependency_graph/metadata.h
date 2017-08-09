#pragma once

#include <string>
#include <functional>
#include <vector>
#include <set>

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include "state.h"
#include "data.h"

namespace dependency_graph {

template<typename T>
class InAttr;

template<typename T>
class OutAttr;

class Values;
class Attr;

class Node;
class Port;

class Metadata : public boost::noncopyable {
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

		/// returns the type of the node this metadata object describes
		const std::string& type() const;

		/// returns true if this metadata instance has no attributes or no compute
		virtual bool isValid() const;


		/// registers an input attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(InAttr<T>& in, const std::string& name, const T& defaultValue = T());

		/// registers an output attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(OutAttr<T>& out, const std::string& name, const T& defaultValue = T());

		/// compute method of this node
		void setCompute(std::function<State(Values&)> compute);

		/// returns the number of attributes currently present
		size_t attributeCount() const;

		/// returns an attribute reference
		Attr& attr(size_t index);
		/// returns an attribute reference
		const Attr& attr(size_t index) const;


		/// adds attribute influence - inputs required to compute outputs
		template<typename T, typename U>
		void addInfluence(const InAttr<T>& in, const OutAttr<U>& out);

		template<typename T>
		std::vector<std::reference_wrapper<const Attr>> influences(const InAttr<T>& in) const;

		template<typename T>
		std::vector<std::reference_wrapper<const Attr>> influencedBy(const OutAttr<T>& out) const;

		/// blind metadata's data, to be used by the client application
		///   to store visual information (e.g., attribute position, colour...)
		template<typename T>
		void setBlindData(const T& value);

		/// blind metadata's data, to be used by the client application
		///   to store visual information (e.g., attribute position, colour...)
		template<typename T>
		const T& blindData() const;


	private:
		std::vector<std::reference_wrapper<const Attr>> influences(size_t index) const;
		std::vector<std::reference_wrapper<const Attr>> influencedBy(size_t index) const;

		std::string m_type;
		std::vector<Attr*> m_attrs; // not owning the attr instances
		std::function<State(Values&)> m_compute;

		boost::bimap<boost::bimaps::multiset_of<unsigned>, boost::bimaps::multiset_of<unsigned>> m_influences;

		std::unique_ptr<BaseData> m_blindData;

		static std::set<Metadata*, Metadata::Comparator> s_instances;

		friend class Node;
		friend class Port;
};

}
