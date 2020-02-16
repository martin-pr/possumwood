#pragma once

#include <string>
#include <iostream>
#include <memory>

namespace lightfields {

/// A block of data in a lightfields raw file
struct Block {
	char id = '\0';
	std::string name;
	std::unique_ptr<unsigned char[]> data;
};

std::istream& operator >> (std::istream& in, Block& block);

}
