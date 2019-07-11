#pragma once

#include <vector>
#include <string>

/// A simple class handling CLI program options.
/// Unfortunately boost::program_options doesn't deal well with options order.
/// This is a simple class that allows evaluation of program options in order.
class Options {
	public:
		struct Item {
			Item(const std::string& n) : name(n) {
			}

			std::string name;
			std::vector<std::string> parameters;
		};

		Options(int argc, char* argv[]);

		typedef std::vector<Item>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

	private:
		std::vector<Item> m_items;
};
