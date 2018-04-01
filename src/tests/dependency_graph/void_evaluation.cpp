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

namespace {

/// scoped handle registration, to allow name and node re-use between tests
class HandleRegistrar : public boost::noncopyable {
	public:
		HandleRegistrar(std::unique_ptr<Metadata>&& meta) : m_handle(new MetadataHandle(std::move(meta))) {
			dependency_graph::MetadataRegister::singleton().add(*m_handle);
		}

		HandleRegistrar(HandleRegistrar&& reg) : m_handle(std::move(reg.m_handle)) {
		}

		~HandleRegistrar() {
			if(m_handle != nullptr)
				dependency_graph::MetadataRegister::singleton().remove(*m_handle);
		}

		operator const MetadataHandle&() const {
			return *m_handle;
		}

	private:
		std::unique_ptr<MetadataHandle> m_handle;
};

// creates a very simple generic one-in-one-out node
template<typename INPUT, typename OUTPUT>
const HandleRegistrar typedNode(
	std::function<void(dependency_graph::Values& val, const InAttr<INPUT>& in, const OutAttr<OUTPUT>& out)> compute) {

	std::unique_ptr<Metadata> meta(new Metadata(std::string(typeid(INPUT).name()) + "_" + typeid(OUTPUT).name()));

	// create attributes
	InAttr<INPUT> input;
	meta->addAttribute(input, "input");

	OutAttr<OUTPUT> output;
	meta->addAttribute(output, "output");

	meta->addInfluence(input, output);

	meta->setCompute([compute, input, output](dependency_graph::Values& vals) {
		compute(vals, input, output);

		return dependency_graph::State();
	});

	return HandleRegistrar(std::move(meta));
}

// a trivial templated compute method, to allow swapping void and non-void inputs/outputs
template<typename INPUT, typename OUTPUT, typename TYPE = float>
struct Assign {
	static void compute(dependency_graph::Values& val, const InAttr<INPUT>& in, const OutAttr<OUTPUT>& out) {
		val.set<TYPE>(out, val.get<TYPE>(in));
	}
};

}

// untyped OUTPUT port evaluation:
//   - simple - compute() produces float coming from input
//   - untyped port's type determine via a value connection (not readable otherwise)
//       - (float, void) -> (float, float)
//   - should evaluate correctly both on the output itself, and indirectly
BOOST_AUTO_TEST_CASE(void_simple_out) {
	Graph g;

	// make a simple assignment node handle, float "float" to "untyped"
	const HandleRegistrar& voidHandle = typedNode<float, void>(Assign<float, void>::compute);
	const HandleRegistrar& floatHandle = typedNode<float, float>(Assign<float, float>::compute);

	NodeBase& voidNode = g.nodes().add(voidHandle, "void_node");
	NodeBase& floatNode = g.nodes().add(floatHandle, "float_node");

	BOOST_REQUIRE(voidNode.portCount() == 2);
	BOOST_REQUIRE(voidNode.port(0).type() == unmangledTypeId<float>());
	BOOST_REQUIRE(voidNode.port(1).type() == unmangledTypeId<void>());

	BOOST_REQUIRE(floatNode.portCount() == 2);
	BOOST_REQUIRE(floatNode.port(0).type() == unmangledTypeId<float>());
	BOOST_REQUIRE(floatNode.port(1).type() == unmangledTypeId<float>());


	// what about getting a value BEFORE connection, should that work?
	// can compute() decide on the output type? NO! Because that would
	// mean that it can render connections invalid by changing type,
	// and that would be sad.


	// the void output doesn't have a type yet as its not connected - reading should throw
	BOOST_CHECK_THROW(voidNode.port(1).get<float>(), std::runtime_error);

	// connect the two nodes together, to "assign a type" to the void node output
	BOOST_REQUIRE_NO_THROW(voidNode.port(1).connect(floatNode.port(0)));

	// the void output now has a type and can be read
	BOOST_CHECK_NO_THROW(voidNode.port(1).get<float>());
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);

	// pull on the float node output (this should just work)
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 0.0f);

	// assign a new value to the void node input
	BOOST_REQUIRE_NO_THROW(voidNode.port(0).set(5.0f));
	// check dirty propagation
	BOOST_CHECK(voidNode.port(1).isDirty());
	BOOST_CHECK(floatNode.port(0).isDirty());
	BOOST_CHECK(floatNode.port(1).isDirty());

	// and check the output on both nodes (i.e., via pull, both directly and indirectly)
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 5.0f);
	// and check nothing stayed dirty after evaluation
	BOOST_CHECK(not voidNode.port(0).isDirty());
	BOOST_CHECK(not voidNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.port(0).isDirty());
	BOOST_CHECK(not floatNode.port(1).isDirty());
}

// 1. untyped OUTPUT port evaluation on error:
// 	 throw an exception during evaluation (pulling on a connected output should work)

// 2. untyped OUTPUT incorrect type request:
//   assign incorrect datatype (should throw an exception, but pulling on the connected out should work)

namespace {

void untyped_handle_error_test(const HandleRegistrar& voidHandle) {
	Graph g;

	const HandleRegistrar& floatHandle = typedNode<float, float>(Assign<float, float>::compute);

	NodeBase& voidNode = g.nodes().add(voidHandle, "void_node");
	NodeBase& floatNode = g.nodes().add(floatHandle, "float_node");

	BOOST_REQUIRE(voidNode.portCount() == 2);
	BOOST_REQUIRE(voidNode.port(0).type() == unmangledTypeId<float>());
	BOOST_REQUIRE(voidNode.port(1).type() == unmangledTypeId<void>());

	BOOST_REQUIRE(floatNode.portCount() == 2);
	BOOST_REQUIRE(floatNode.port(0).type() == unmangledTypeId<float>());
	BOOST_REQUIRE(floatNode.port(1).type() == unmangledTypeId<float>());

	// the void output doesn't have a type yet as its not connected - reading should throw
	BOOST_CHECK_THROW(voidNode.port(1).get<float>(), std::runtime_error);

	// connect the two nodes together, to "assign a type" to the void node output
	BOOST_REQUIRE_NO_THROW(voidNode.port(1).connect(floatNode.port(0)));

	// the void output now has a type and can be read
	BOOST_CHECK_NO_THROW(voidNode.port(1).get<float>());
	// pull on the float node output (this should work, but return default-constructed float)
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 0.0f);

	// but compute() thrown an exception, so the state of the node should be invalid
	BOOST_CHECK(voidNode.state().errored());
	BOOST_CHECK(not floatNode.state().errored());

	// assign a new value to the void node input
	BOOST_REQUIRE_NO_THROW(voidNode.port(0).set(5.0f));
	// check dirty propagation
	BOOST_CHECK(voidNode.port(1).isDirty());
	BOOST_CHECK(floatNode.port(0).isDirty());
	BOOST_CHECK(floatNode.port(1).isDirty());

	// and check the output on both nodes (i.e., via pull, both directly and indirectly)
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	// // and check nothing stayed dirty after evaluation
	BOOST_CHECK(not voidNode.port(0).isDirty());
	BOOST_CHECK(not voidNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.port(0).isDirty());
	BOOST_CHECK(not floatNode.port(1).isDirty());
}

}

// 1. untyped OUTPUT port evaluation on error:
// 	 throw an exception during evaluation (pulling on a connected output should work)
BOOST_AUTO_TEST_CASE(void_error_out) {
	// make a simple assignment node handle, float "float" to "untyped"
	const HandleRegistrar& voidHandle = typedNode<float, void>(
		[](dependency_graph::Values& val, const InAttr<float>& in, const OutAttr<void>& out) {
			throw std::runtime_error("error");
		}
	);

	untyped_handle_error_test(voidHandle);
}

// 2. untyped OUTPUT incorrect type request:
//   assign incorrect datatype (should throw an exception, but pulling on the connected out should work)
BOOST_AUTO_TEST_CASE(void_error_assignment_out) {
	// make a simple assignment node handle, float "float" to "untyped"
	const HandleRegistrar& voidHandle = typedNode<float, void>(
		[](dependency_graph::Values& val, const InAttr<float>& in, const OutAttr<void>& out) {
			// assigns an integer - assuming connected node is float, this should throw an exception
			val.set(out, (int)val.get(in));
		}
	);

	untyped_handle_error_test(voidHandle);
}

// untyped OUTPUT port evaluation:
//   - more complex - if output is connected to a float, produce float; if to int, produce int; error otherwise

// untyped INPUT port evaluation:
//   - simple - compute() takes a float

// untyped INPUT port evaluation on error:
// 	 throw an exception during evaluation

// untyped INPUT incorrect type request:
//   get() incorrect datatype (should throw an exception, but pulling on the connected out should work)

// untyped INPUT connection with compute() having wrong type
//   - connecting an int errors on evaluation on type test, when compute() requests a float

// untyped INPUT port evaluation:
// same as above, but via a connected node: (float, float) -> (void, float)

// untyped INPUT port evaluation:
//   - more complex - based on the input type, do evaluation with the right type, always return a float

// test error with a void pointer - reset() of an out port with void type
//   after compute() throws an exception

