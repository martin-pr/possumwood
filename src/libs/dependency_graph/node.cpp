#include "node.h"

#include <cassert>

#include "graph.h"
#include "values.h"

namespace dependency_graph {

Node::Node(const std::string& name, const UniqueId& id, const MetadataHandle& def, Network* parent) : NodeBase(name, id, def, parent) {
}

}
