#pragma once

#include <dependency_graph/metadata.h>

/// returns a reference to metadata of a simple addition node
const dependency_graph::MetadataHandle& additionNode();

/// returns a reference to metadata of a simple multiplication node
const dependency_graph::MetadataHandle& multiplicationNode();

/// returns a reference to metadata of an int and float addition node, without output as first attr.
/// Mainly usable for testing consistency of metadata mapping.
const dependency_graph::MetadataHandle& intAdditionNode();

/// returns a reference to metadata of a simple float pass-through node
const dependency_graph::MetadataHandle& passThroughNode();
