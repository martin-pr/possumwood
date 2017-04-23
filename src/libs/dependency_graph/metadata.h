#pragma once

#include <string>
#include <functional>
#include <vector>

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

namespace dependency_graph {

template<typename T>
class InAttr;

template<typename T>
class OutAttr;

class Datablock;
class Attr;

class Node;

class Metadata : public boost::noncopyable {
	public:
		Metadata(const std::string& nodeType);

		/// returns the type of the node this metadata object describes
		const std::string& type() const;

		/// returns true if this metadata instance has no attributes or no compute
		bool isValid() const;


		/// registers an input attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(InAttr<T>& in, const std::string& name);

		/// registers an output attribute.
		/// Each attribute instance should be held statically in the
		/// implementation of the "node" concept of the target application.
		/// This call does *not* take ownership of the attribute, and assumes
		/// that it will be available throughout the application run.
		template<typename T>
		void addAttribute(OutAttr<T>& out, const std::string& name);

		/// compute method of this node
		void setCompute(std::function<void(Datablock&)> compute);

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

	private:
		std::vector<std::reference_wrapper<const Attr>> influences(size_t index) const;
		std::vector<std::reference_wrapper<const Attr>> influencedBy(size_t index) const;

		std::string m_type;
		std::vector<Attr*> m_attrs; // not owning the attr instances
		std::function<void(Datablock&)> m_compute;

		boost::bimap<boost::bimaps::multiset_of<unsigned>, boost::bimaps::multiset_of<unsigned>> m_influences;

		friend class Node;
};

}
