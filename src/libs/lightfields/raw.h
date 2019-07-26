#pragma once

#include <vector>
#include <iostream>
#include <memory>

#include <json/value.h>

namespace lightfields {

class Raw {
	public:
		Raw();
		~Raw();

		const Json::Value& header() const;
		const Json::Value& metadata() const;
		const Json::Value& privateMetadata() const;

		const std::vector<char> image() const;

	private:
		class Pimpl;

		std::shared_ptr<const Pimpl> m_pimpl;

	friend std::istream& operator >> (std::istream& in, Raw& data);
};

std::istream& operator >> (std::istream& in, Raw& data);

}
