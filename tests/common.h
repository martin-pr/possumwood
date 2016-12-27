#pragma once

#include <iostream>
#include <typeinfo>

namespace std {

/// printing type_info, to allow its use in BOOST_*_EQUAL macro
std::ostream& operator << (std::ostream& out, const std::type_info& t);

}


/// a simple custom struct "data holder" - copyable, and each instance comes with
///   its "own ID"
struct TestStruct {
	TestStruct();

	bool operator == (const TestStruct& ts) const;
	bool operator != (const TestStruct& ts) const;

	unsigned id;
};

std::ostream& operator << (std::ostream& out, const TestStruct& t);
