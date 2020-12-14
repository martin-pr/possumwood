#pragma once

#include <nlohmann/json.hpp>

namespace lightfields {

class Raw;

/// Image metadata holder - instantiated by loading a Raw image, but handled as a separate type in Possumwood
class Metadata {
  public:
	Metadata();

	const nlohmann::json& header() const;
	const nlohmann::json& metadata() const;
	const nlohmann::json& privateMetadata() const;

  private:
	nlohmann::json m_header, m_meta, m_privateMeta;

	friend class Raw;
};

std::ostream& operator<<(std::ostream& out, const Metadata& meta);

}  // namespace lightfields
