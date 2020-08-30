#include "attr_map.h"

#include "attr.inl"

namespace dependency_graph {

AttrMap::AttrMap(const MetadataHandle& src, const MetadataHandle& dest) : m_src(src), m_dest(dest) {
	std::set<unsigned> unmappedSrcAttrs;
	for(std::size_t a = 0; a < src->attributeCount(); ++a)
		unmappedSrcAttrs.insert(a);

	std::set<unsigned> unmappedDestAttrs;
	for(std::size_t a = 0; a < dest->attributeCount(); ++a)
		unmappedDestAttrs.insert(a);

	// first try 1:1 mapping
	for(unsigned ai = 0; ai < std::min(src->attributeCount(), dest->attributeCount()); ++ai) {
		const Attr& sa = src->attr(ai);
		const Attr& da = dest->attr(ai);

		if(sa.name() == da.name() && sa.category() == da.category() && sa.type() == da.type()) {
			m_map.left.insert(std::make_pair(ai, ai));
			unmappedSrcAttrs.erase(unmappedSrcAttrs.find(ai));
			unmappedDestAttrs.erase(unmappedDestAttrs.find(ai));
		}
	}

	// next, try precise matching
	{
		auto srcIt = unmappedSrcAttrs.begin();
		while(srcIt != unmappedSrcAttrs.end()) {
			const Attr& srcAttr = m_src->attr(*srcIt);

			auto destIt = unmappedDestAttrs.end();
			for(auto i = unmappedDestAttrs.begin(); i != unmappedDestAttrs.end(); ++i) {
				const Attr& destAttr = m_dest->attr(*i);
				if(srcAttr.name() == destAttr.name() && srcAttr.category() == destAttr.category() &&
				   srcAttr.type() == destAttr.type()) {
					destIt = i;
					break;
				}
			}

			if(destIt != unmappedDestAttrs.end()) {
				m_map.left.insert(std::make_pair(*srcIt, *destIt));

				srcIt = unmappedSrcAttrs.erase(srcIt);
				destIt = unmappedDestAttrs.erase(destIt);
			}
			else
				++srcIt;
		}
	}

	// "name has changed" mapping (exclude name from the identity testing)
	{
		auto srcIt = unmappedSrcAttrs.begin();
		while(srcIt != unmappedSrcAttrs.end()) {
			const Attr& srcAttr = m_src->attr(*srcIt);

			auto destIt = unmappedDestAttrs.end();
			for(auto i = unmappedDestAttrs.begin(); i != unmappedDestAttrs.end(); ++i) {
				const Attr& destAttr = m_dest->attr(*i);
				if(srcAttr.category() == destAttr.category() && srcAttr.type() == destAttr.type()) {
					destIt = i;
					break;
				}
			}

			if(destIt != unmappedDestAttrs.end()) {
				m_map.left.insert(std::make_pair(*srcIt, *destIt));

				srcIt = unmappedSrcAttrs.erase(srcIt);
				destIt = unmappedDestAttrs.erase(destIt);
			}
			else
				++srcIt;
		}
	}
}

const boost::optional<const Attr&> AttrMap::srcToDest(const Attr& srcAttr) const {
	auto it = m_map.left.find(srcAttr.offset());
	if(it != m_map.left.end())
		return m_dest->attr(it->second);
	else
		return boost::optional<const Attr&>();
}

const boost::optional<const Attr&> AttrMap::destToSrc(const Attr& destAttr) const {
	auto it = m_map.right.find(destAttr.offset());
	if(it != m_map.right.end())
		return m_dest->attr(it->second);
	else
		return boost::optional<const Attr&>();
}

const MetadataHandle& AttrMap::source() const {
	return m_src;
}

const MetadataHandle& AttrMap::destination() const {
	return m_dest;
}

AttrMap::const_iterator AttrMap::begin() const {
	return m_map.left.begin();
}

AttrMap::const_iterator AttrMap::end() const {
	return m_map.left.end();
}

AttrMap::const_iterator AttrMap::find(unsigned srcPort) const {
	return m_map.left.find(srcPort);
}

bool AttrMap::empty() const {
	return m_map.empty();
}

std::size_t AttrMap::size() const {
	return m_map.size();
}

}  // namespace dependency_graph
