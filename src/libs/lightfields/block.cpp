#include "block.h"

namespace lightfields {

std::istream& operator>>(std::istream& in, Block& block) {
	// read the header
	block.id = '\0';
	block.name.clear();

	unsigned char header[8];
	in.read((char*)header, 8);

	if(in.eof())
		return in;

	if(header[0] != 0x89 || header[1] != 'L' || header[2] != 'F')
		throw std::runtime_error("Lytro file magic sequence not matching - wrong file type?");

	block.id = header[3];

	// skip the version
	in.seekg(4, in.cur);

	// read the big endian length
	std::size_t length = 0;
	for(std::size_t a = 0; a < 4; ++a)
		length = (length << 8) + (unsigned char)in.get();

	if(block.id != 'P') {
		// read the name
		{
			char name[81];
			in.read(name, 80);
			name[80] = '\0';
			block.name = name;
		}

		// and read the data block
		block.data = std::unique_ptr<unsigned char[]>(new unsigned char[length + 1]);
		in.read((char*)block.data.get(), length);
		block.data[length] = '\0';

		// handle any padding
		while(in.tellg() % 16 != 0)
			in.get();
	}

	return in;
}

}  // namespace lightfields
