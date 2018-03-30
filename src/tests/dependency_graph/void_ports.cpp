#include <boost/test/unit_test.hpp>

#include <dependency_graph/graph.h>
#include <dependency_graph/node.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/metadata_register.h>
#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>

#include "common.h"

using namespace dependency_graph;

template<typename T>
const dependency_graph::MetadataHandle& typedInput() {
	static std::unique_ptr<MetadataHandle> s_handle;

	if(s_handle == nullptr) {
		std::unique_ptr<Metadata> meta(new Metadata(std::string(typeid(T).name()) + "_input"));

		// create attributes
		static InAttr<T> input;
		meta->addAttribute(input, "input");

		s_handle = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(meta)));

		dependency_graph::MetadataRegister::singleton().add(*s_handle);
	}

	return *s_handle;
}

const dependency_graph::MetadataHandle& voidOutput() {
	static std::unique_ptr<MetadataHandle> s_handle;

	if(s_handle == nullptr) {
		std::unique_ptr<Metadata> meta(new Metadata("void_output"));

		// create attributes
		OutAttr<void> output;
		meta->addAttribute(output, "output");

		s_handle = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(meta)));

		dependency_graph::MetadataRegister::singleton().add(*s_handle);
	}

	return *s_handle;
}

BOOST_AUTO_TEST_CASE(void_input_ports) {
	Graph g;

	// make 2 nodes - add and void
	const MetadataHandle& additionHandle = additionNode();
	const MetadataHandle& voidInputHandle = typedInput<void>();

	NodeBase& add1 = g.nodes().add(additionHandle, "add_1");
	NodeBase& void1 = g.nodes().add(voidInputHandle, "in_void_1");

	BOOST_CHECK(add1.portCount() == 3);
	BOOST_CHECK(void1.portCount() == 1);

	BOOST_REQUIRE_EQUAL(void1.port(0).type(), unmangledTypeId<void>());
	BOOST_REQUIRE_EQUAL(void1.port(0).category(), dependency_graph::Attr::kInput);

	BOOST_REQUIRE_EQUAL(add1.port(2).type(), unmangledTypeId<float>());
	BOOST_REQUIRE_EQUAL(add1.port(2).category(), dependency_graph::Attr::kOutput);

	// setting or getting a value of a void port should throw an exception
	BOOST_CHECK_THROW(void1.port(0).set<float>(5.0f), std::runtime_error);
	BOOST_CHECK_THROW(void1.port(0).get<float>(), std::runtime_error);

	// try to connect
	BOOST_REQUIRE_NO_THROW(add1.port(2).connect(void1.port(0)));
	BOOST_REQUIRE(add1.port(2).isConnected());
	BOOST_REQUIRE(void1.port(0).isConnected());
	BOOST_REQUIRE(g.connections().connectedFrom(void1.port(0)));
	BOOST_REQUIRE_EQUAL(&(*g.connections().connectedFrom(void1.port(0))), &(add1.port(2)));

	// check the result of the connection
	BOOST_REQUIRE_EQUAL(void1.port(0).type(), unmangledTypeId<float>());
	BOOST_REQUIRE_EQUAL(void1.port(0).category(), dependency_graph::Attr::kInput);

	// try to get a value from it
	BOOST_REQUIRE_EQUAL(void1.port(0).get<float>(), 0.0f);

	// change the input value of the addition node, and check it propagated
	BOOST_REQUIRE_NO_THROW(add1.port(0).set<float>(5.0f));
	BOOST_CHECK(add1.port(2).isDirty());
	BOOST_CHECK(void1.port(0).isDirty());

	// and check the value
	BOOST_REQUIRE_EQUAL(void1.port(0).get<float>(), 5.0f);
	BOOST_CHECK(not add1.port(2).isDirty());
	BOOST_CHECK(not void1.port(0).isDirty());

	// disconnect
	BOOST_REQUIRE_NO_THROW(add1.port(2).disconnect(void1.port(0)));
	BOOST_CHECK(not void1.port(0).isConnected());
	BOOST_CHECK(not add1.port(2).isConnected());
	BOOST_CHECK(not g.connections().connectedFrom(void1.port(0)));

	 // + test that its type returns back to "void" after disconnect
	BOOST_REQUIRE_EQUAL(void1.port(0).type(), unmangledTypeId<void>());
	BOOST_REQUIRE_EQUAL(void1.port(0).category(), dependency_graph::Attr::kInput);

	BOOST_CHECK_THROW(void1.port(0).get<float>(), std::runtime_error);
	BOOST_CHECK_THROW(void1.port(0).set<float>(5.0f), std::runtime_error);
	BOOST_CHECK_THROW(void1.port(0).get<float>(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(void_output_ports) {
	Graph g;

	// make 2 nodes - add and void
	const MetadataHandle& additionHandle = additionNode();
	const MetadataHandle& voidInputHandle = voidOutput();

	NodeBase& add1 = g.nodes().add(additionHandle, "add_1");
	NodeBase& void1 = g.nodes().add(voidInputHandle, "out_void_1");

	BOOST_CHECK(add1.portCount() == 3);
	BOOST_CHECK(void1.portCount() == 1);

	BOOST_REQUIRE_EQUAL(void1.port(0).type(), unmangledTypeId<void>());
	BOOST_REQUIRE_EQUAL(void1.port(0).category(), dependency_graph::Attr::kOutput);

	BOOST_REQUIRE_EQUAL(add1.port(0).type(), unmangledTypeId<float>());
	BOOST_REQUIRE_EQUAL(add1.port(0).category(), dependency_graph::Attr::kInput);

	// setting or getting a value of a void port should throw an exception
	BOOST_CHECK_THROW(void1.port(0).set<float>(5.0f), std::runtime_error);
	BOOST_CHECK_THROW(void1.port(0).get<float>(), std::runtime_error);

	// try to connect
	BOOST_REQUIRE_NO_THROW(void1.port(0).connect(add1.port(0)));
	BOOST_REQUIRE(add1.port(0).isConnected());
	BOOST_REQUIRE(void1.port(0).isConnected());
	BOOST_REQUIRE(g.connections().connectedFrom(add1.port(0)));
	BOOST_REQUIRE(not g.connections().connectedTo(void1.port(0)).empty());
	BOOST_REQUIRE_EQUAL(&(*g.connections().connectedFrom(add1.port(0))), &(void1.port(0)));

	// check the result of the connection
	BOOST_REQUIRE_EQUAL(void1.port(0).type(), unmangledTypeId<float>());
	BOOST_REQUIRE_EQUAL(void1.port(0).category(), dependency_graph::Attr::kOutput);

	// disconnect
	BOOST_REQUIRE_NO_THROW(void1.port(0).disconnect(add1.port(0)));
	BOOST_CHECK(not void1.port(0).isConnected());
	BOOST_CHECK(not add1.port(0).isConnected());
	BOOST_CHECK(g.connections().connectedTo(void1.port(0)).empty());

	 // + test that its type returns back to "void" after disconnect
	BOOST_REQUIRE_EQUAL(void1.port(0).type(), unmangledTypeId<void>());
	BOOST_REQUIRE_EQUAL(void1.port(0).category(), dependency_graph::Attr::kOutput);

	BOOST_CHECK_THROW(void1.port(0).get<float>(), std::runtime_error);
	BOOST_CHECK_THROW(void1.port(0).set<float>(5.0f), std::runtime_error);
	BOOST_CHECK_THROW(void1.port(0).get<float>(), std::runtime_error);
}

// it should not be possible to connect void out to void in
BOOST_AUTO_TEST_CASE(void_to_void_connection) {
	Graph g;

	// make 2 nodes - add and void
	const MetadataHandle& inHandle = typedInput<void>();
	const MetadataHandle& outHandle = voidOutput();

	NodeBase& in = g.nodes().add(inHandle, "in_void");
	NodeBase& out = g.nodes().add(outHandle, "out_void");

	BOOST_CHECK(in.portCount() == 1);
	BOOST_CHECK(out.portCount() == 1);

	BOOST_REQUIRE_EQUAL(out.port(0).type(), unmangledTypeId<void>());
	BOOST_REQUIRE_EQUAL(out.port(0).category(), dependency_graph::Attr::kOutput);

	BOOST_REQUIRE_EQUAL(in.port(0).type(), unmangledTypeId<void>());
	BOOST_REQUIRE_EQUAL(in.port(0).category(), dependency_graph::Attr::kInput);

	BOOST_CHECK_THROW(out.port(0).connect(in.port(0)), std::runtime_error);
}

// void output port can be connected to multiple different inputs, with different types
//   -> this should throw an error
BOOST_AUTO_TEST_CASE(multi_typed_to_void_connection) {
	Graph g;

	// make 2 nodes - add and void
	const MetadataHandle& inFloatHandle = typedInput<float>();
	const MetadataHandle& inIntHandle = typedInput<int>();
	const MetadataHandle& outHandle = voidOutput();

	NodeBase& inFloat = g.nodes().add(inFloatHandle, "in_float");
	NodeBase& inInt = g.nodes().add(inIntHandle, "in_int");
	NodeBase& out = g.nodes().add(outHandle, "out_void");

	BOOST_CHECK(inFloat.portCount() == 1);
	BOOST_CHECK(inInt.portCount() == 1);
	BOOST_CHECK(out.portCount() == 1);

	BOOST_REQUIRE_EQUAL(out.port(0).type(), unmangledTypeId<void>());
	BOOST_REQUIRE_EQUAL(out.port(0).category(), dependency_graph::Attr::kOutput);

	BOOST_REQUIRE_EQUAL(inFloat.port(0).type(), unmangledTypeId<float>());
	BOOST_REQUIRE_EQUAL(inFloat.port(0).category(), dependency_graph::Attr::kInput);

	BOOST_REQUIRE_EQUAL(inInt.port(0).type(), unmangledTypeId<int>());
	BOOST_REQUIRE_EQUAL(inInt.port(0).category(), dependency_graph::Attr::kInput);

	// connect the first one
	BOOST_CHECK_EQUAL(out.port(0).type(), unmangledTypeId<void>());

	BOOST_REQUIRE_NO_THROW(out.port(0).connect(inFloat.port(0)));

	BOOST_CHECK_EQUAL(out.port(0).type(), unmangledTypeId<float>());

	// connecting another type should throw an exception
	BOOST_CHECK_THROW(out.port(0).connect(inInt.port(0)), std::runtime_error);

	// now, disconnect the original connection
	BOOST_REQUIRE_NO_THROW(out.port(0).disconnect(inFloat.port(0)));

	BOOST_CHECK_EQUAL(out.port(0).type(), unmangledTypeId<void>());

	// connecting the int input should work now
	BOOST_CHECK_NO_THROW(out.port(0).connect(inInt.port(0)));

	BOOST_CHECK_EQUAL(out.port(0).type(), unmangledTypeId<int>());
}


// untyped OUTPUT port evaluation:
//   - simple - compute() produces float coming from input
//   - more complex - if output is connected to a float, produce float; if to int, produce int; error otherwise

// untyped INPUT port evaluation:
//   - simple - compute() takes a float; connecting an int errors on evaluation on type test
//   - more complex - based on the input type, do evaluation with the right type, always return a float

// test error with a void pointer - reset() of an out port with void type
//   after compute() throws an exception

// need to add dirty querying on dependency_graph::Values, to allow partial evaluation
//   -> allows to do a multiple-output multiple-input complex node (or NETWORK)
//   + TESTS
