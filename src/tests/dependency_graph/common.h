#pragma once

#include <iostream>
#include <typeinfo>

#include <dependency_graph/metadata.h>

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

//////

/// returns a reference to metadata of a simple addition node
const dependency_graph::Metadata& additionNode();

/// returns a reference to metadata of a simple multiplication node
const dependency_graph::Metadata& multiplicationNode();
