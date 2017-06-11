#include "node_implementation.h"

NodeImplementation::NodeImplementation(const std::string& nodeName, std::function<void(Metadata&)> init) : m_meta(nodeName) {
	init(m_meta);
}
