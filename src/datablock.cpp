#include "datablock.h"

#include "metadata.h"
#include "attr.h"

Datablock::Datablock(const Metadata& meta) {
	for(std::size_t a=0; a<meta.attributeCount(); ++a)
		m_data.push_back(std::move(meta.attr(a).createData()));
}
