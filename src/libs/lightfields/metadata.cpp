#include "metadata.h"

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

}
