#pragma once

#include <boost/bimap.hpp>
#include <boost/optional.hpp>

#include "metadata.h"

namespace dependency_graph {

/// a simple map class, determining which attributes are to be mapped between two metadata instances.
class AttrMap {
	public:
		/// Returns a map structure between ports of the two nodes, as they
		/// are going to be mapped.
		AttrMap(const MetadataHandle& src, const MetadataHandle& dest);


		const boost::optional<const
		Attr&> srcToDest(const Attr& srcAttr) const;
		const boost::optional<const Attr&> destToSrc(const Attr& destAttr) const;

		const MetadataHandle& source() const;
		const MetadataHandle& destination() const;

		typedef boost::bimap<unsigned, unsigned>::left_const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		bool empty() const;
		std::size_t size() const;

	private:
		MetadataHandle m_src, m_dest;
		boost::bimap<unsigned, unsigned> m_map;
};

}
