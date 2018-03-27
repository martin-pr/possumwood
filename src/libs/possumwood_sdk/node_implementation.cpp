#include "node_implementation.h"

#include <dependency_graph/metadata_register.h>

namespace possumwood {

NodeImplementation::NodeImplementation(const std::string& nodeName,
                                       std::function<void(Metadata&)> init) : m_meta(std::unique_ptr<Metadata>(new Metadata(nodeName))) {
	std::unique_ptr<Metadata> meta(new Metadata(nodeName));

	init(*meta);

	m_meta = dependency_graph::MetadataHandle(std::move(meta));

	dependency_graph::MetadataRegister::singleton().add(m_meta);
}

NodeImplementation::~NodeImplementation() {
	dependency_graph::MetadataRegister::singleton().remove(m_meta);
}

}
