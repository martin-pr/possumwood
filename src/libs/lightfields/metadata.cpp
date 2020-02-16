#include "metadata.h"

#include <json/writer.h>

namespace lightfields {

Metadata::Metadata() {
}

const Json::Value& Metadata::header() const {
	return m_header;
}
const Json::Value& Metadata::metadata() const {
	return m_meta;
}
const Json::Value& Metadata::privateMetadata() const {
	return m_privateMeta;
}

std::ostream& operator << (std::ostream& out, const Metadata& meta) {
	out << "*** Lytro metadata: ***" << std::endl;
	out << std::endl;

	Json::StyledWriter writer;

	out << "### Header" << std::endl;
	out << writer.write(meta.header()) << std::endl;

	out << "### Metadata" << std::endl;
	out << writer.write(meta.metadata()) << std::endl;

	out << "### Private metadata:" << std::endl;
	out << writer.write(meta.privateMetadata()) << std::endl;

	return out;
}

}
