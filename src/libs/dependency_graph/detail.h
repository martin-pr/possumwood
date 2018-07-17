#pragma once

#include "metadata.h"

namespace dependency_graph { namespace detail {

/// A simple accessor, allowing to add an untemplated attribute
/// to a metadata instance. Only to be used in Actions implementation.
struct MetadataAccess {
	static void addAttribute(Metadata& meta, Attr& attr) {
		meta.doAddAttribute(attr);
	}
};

} }
