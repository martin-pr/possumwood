#pragma once

#include <functional>

class Datablock;

class NodeType {
	public:
		NodeType(const Metadata& meta, std::function<bool(Datablock&)> compute);
};
