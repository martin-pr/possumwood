#include "metadata.h"

#include <iomanip>

#include <nlohmann/json.hpp>

namespace lightfields {

Metadata::Metadata() {
}

const nlohmann::json& Metadata::header() const {
	return m_header;
}
const nlohmann::json& Metadata::metadata() const {
	return m_meta;
}
const nlohmann::json& Metadata::privateMetadata() const {
	return m_privateMeta;
}

std::ostream& operator<<(std::ostream& out, const Metadata& meta) {
	out << "*** Lytro metadata: ***" << std::endl;
	out << std::endl;

	out << std::setw(4);

	out << "### Header" << std::endl;
	out << meta.header() << std::endl;

	out << "### Metadata" << std::endl;
	out << meta.metadata() << std::endl;

	out << "### Private metadata:" << std::endl;
	out << meta.privateMetadata() << std::endl;

	return out;
}

}  // namespace lightfields
