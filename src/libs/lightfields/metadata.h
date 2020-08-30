#pragma once

#include <json/value.h>

namespace lightfields {

class Raw;

/// Image metadata holder - instantiated by loading a Raw image, but handled as a separate type in Possumwood
class Metadata {
  public:
	Metadata();

	const Json::Value& header() const;
	const Json::Value& metadata() const;
	const Json::Value& privateMetadata() const;

  private:
	Json::Value m_header, m_meta, m_privateMeta;

	friend class Raw;
};

std::ostream& operator<<(std::ostream& out, const Metadata& meta);

}  // namespace lightfields
