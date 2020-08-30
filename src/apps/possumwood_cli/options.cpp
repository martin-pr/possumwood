#include "options.h"

#include <boost/algorithm/string/predicate.hpp>
#include <iostream>

Options::Options(int argc, char* argv[]) {
	for(int current = 1; current < argc; ++current) {
		if(boost::starts_with(argv[current], "--"))
			m_items.push_back(Item(argv[current]));

		else if(!m_items.empty())
			m_items.back().parameters.push_back(argv[current]);

		else
			throw std::runtime_error("Command line parameters must start with '--'");
	}
}

Options::const_iterator Options::begin() const {
	return m_items.begin();
}

Options::const_iterator Options::end() const {
	return m_items.end();
}
