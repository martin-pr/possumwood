#pragma once

#include "metadata.h"
#include "attr.h"

namespace dependency_graph { namespace detail {

/// A simple accessor, allowing to add an untemplated attribute
/// to a metadata instance. Only to be used in Actions implementation.
struct MetadataAccess {
	static void addAttribute(Metadata& meta, Attr& attr) {
		meta.doAddAttribute(attr);
	}

	static void addAttribute(Metadata& meta, const std::string& name, Attr::Category cat, const Data& data, unsigned flags) {
		meta.doAddAttribute(name, cat, data, flags);
	}
};

} }
