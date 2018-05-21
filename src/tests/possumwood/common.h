#pragma once

#include <dependency_graph/metadata.h>

/// returns a reference to metadata of a simple addition node
const dependency_graph::MetadataHandle& additionNode();

/// returns a reference to metadata of a simple multiplication node
const dependency_graph::MetadataHandle& multiplicationNode();
