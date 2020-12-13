#pragma once

#include <nlohmann/json.hpp>

namespace lightfields {

class Bayer {
  public:
	struct Value {
		Value();
		Value(const nlohmann::json& json);

		uint16_t operator[](unsigned id) const;

		uint16_t b, gb, gr, r;
	};

  private:
};

}  // namespace lightfields
