#pragma once

#include <functional>

class Datablock;

class NodeDefinition {
	public:
		NodeDefinition(const Metadata& meta, std::function<bool(Datablock&)> compute);
};
