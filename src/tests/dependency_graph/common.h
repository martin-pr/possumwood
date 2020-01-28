#pragma once

#include <iostream>
#include <typeinfo>

#include <dependency_graph/metadata.h>
#include <actions/io/json.h>

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

// implementation of serialization of TestStruct - to make tests compile
inline void to_json(::possumwood::io::json& json, const TestStruct& ts) {}
inline void from_json(const ::possumwood::io::json& json, TestStruct& ts) {}

std::ostream& operator << (std::ostream& out, const TestStruct& t);

//////

/// a simple noncopyable struct "data holder" - noncopyable, movable, and each instance comes with
///   its "own ID".
/// This struct also doesn't have a comparison operator - reassignment (and recomputation) is always needed
struct NoncopyableStruct {
	NoncopyableStruct();

	NoncopyableStruct(const NoncopyableStruct&) = delete;
	NoncopyableStruct& operator = (const NoncopyableStruct&) = delete;

	NoncopyableStruct(NoncopyableStruct&&) = default;
	NoncopyableStruct& operator = (NoncopyableStruct&&) = default;

	unsigned id;
};

// no implementation of serialization - intentional, to test support for types that can't be serialized
//inline void to_json(::possumwood::io::json& json, const TestStruct& ts) {}
//inline void from_json(const ::possumwood::io::json& json, TestStruct& ts) {}

std::ostream& operator << (std::ostream& out, const NoncopyableStruct& t);

//////

/// returns a reference to metadata of a simple addition node
const dependency_graph::MetadataHandle& additionNode();

/// returns a reference to metadata of a simple multiplication node
const dependency_graph::MetadataHandle& multiplicationNode();
