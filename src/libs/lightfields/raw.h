#pragma once

#include <nlohmann/json.hpp>

#include <iostream>
#include <memory>
#include <vector>

namespace lightfields {

class Metadata;

class Raw {
  public:
	Raw();
	~Raw();

	const Metadata& metadata() const;

	const unsigned char* image() const;

  private:
	struct Pimpl;

	// only used for file reading
	static nlohmann::json& header(Pimpl& p);
	static nlohmann::json& meta(Pimpl& p);
	static nlohmann::json& privateMeta(Pimpl& p);

	std::shared_ptr<const Pimpl> m_pimpl;

	friend std::istream& operator>>(std::istream& in, Raw& data);
};

std::istream& operator>>(std::istream& in, Raw& data);

}  // namespace lightfields
