#pragma once

#include <json/value.h>

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
	static Json::Value& header(Pimpl& p);
	static Json::Value& meta(Pimpl& p);
	static Json::Value& privateMeta(Pimpl& p);

	std::shared_ptr<const Pimpl> m_pimpl;

	friend std::istream& operator>>(std::istream& in, Raw& data);
};

std::istream& operator>>(std::istream& in, Raw& data);

}  // namespace lightfields
