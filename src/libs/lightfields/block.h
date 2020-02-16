#pragma once

#include <string>
#include <vector>
#include <iostream>

namespace lightfields {

/// A block of data in a lightfields raw file
struct Block {
	char id = '\0';
	std::string name;
	std::vector<unsigned char> data;
};

std::istream& operator >> (std::istream& in, Block& block);

}
