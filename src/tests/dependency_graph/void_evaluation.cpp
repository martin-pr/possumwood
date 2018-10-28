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
HandleRegistrar typedNode(
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
	BOOST_CHECK(not voidNode.state().errored());

	// pull on the float node output (this should just work)
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK(not voidNode.state().errored());

	// assign a new value to the void node input
	BOOST_REQUIRE_NO_THROW(voidNode.port(0).set(5.0f));
	BOOST_CHECK(not voidNode.state().errored());
	// check dirty propagation
	BOOST_CHECK(voidNode.port(1).isDirty());
	BOOST_CHECK(floatNode.port(0).isDirty());
	BOOST_CHECK(floatNode.port(1).isDirty());
	BOOST_CHECK(not voidNode.state().errored());

	// and check the output on both nodes (i.e., via pull, both directly and indirectly)
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 5.0f);
	BOOST_CHECK(not voidNode.state().errored());
	// and check nothing stayed dirty after evaluation
	BOOST_CHECK(not voidNode.port(0).isDirty());
	BOOST_CHECK(not voidNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.port(0).isDirty());
	BOOST_CHECK(not floatNode.port(1).isDirty());
	BOOST_CHECK(not voidNode.state().errored());
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
	BOOST_CHECK(voidNode.state().errored());

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
	BOOST_CHECK(not floatNode.state().errored());
	BOOST_CHECK(voidNode.state().errored());

	// and check the output on both nodes (i.e., via pull, both directly and indirectly)
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK(not floatNode.state().errored());
	BOOST_CHECK(voidNode.state().errored());
	// // and check nothing stayed dirty after evaluation
	BOOST_CHECK(not voidNode.port(0).isDirty());
	BOOST_CHECK(not voidNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.port(0).isDirty());
	BOOST_CHECK(not floatNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.state().errored());
	BOOST_CHECK(voidNode.state().errored());
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
BOOST_AUTO_TEST_CASE(void_complex_out) {
	Graph g;

	// apart from simple assignment nodes, make one node that can handle both
	//   float and int, depending on what is connected to it (and throws an exception
	//   on an unknown type)
	const MetadataHandle voidHandle = typedNode<float, void>(
		[](dependency_graph::Values& val, const InAttr<float>& in, const OutAttr<void>& out) {
			// based on type, do the right thing or throw
			if(val.is<float>(out))
				val.set(out, val.get(in));
			else if(val.is<int>(out))
				val.set(out, (int)val.get(in));
			else
				throw std::runtime_error("unknown");
		}
	);
	const MetadataHandle floatHandle = typedNode<float, float>(Assign<float, float>::compute);
	const MetadataHandle intHandle = typedNode<int, int>(Assign<int, int, int>::compute);
	const MetadataHandle unsignedHandle = typedNode<unsigned, unsigned>(Assign<unsigned, unsigned, unsigned>::compute);

	NodeBase& voidNode = g.nodes().add(voidHandle, "void_node");
	NodeBase& floatNode = g.nodes().add(floatHandle, "float_node");
	NodeBase& intNode = g.nodes().add(intHandle, "int_node");
	NodeBase& unsignedNode = g.nodes().add(unsignedHandle, "unsigned_node");

	// first, lets try the void node on its own, with no connections
	BOOST_CHECK_THROW(voidNode.port(1).get<float>(), std::runtime_error);

	// connect float, try to pull
	BOOST_REQUIRE_NO_THROW(voidNode.port(1).connect(floatNode.port(0)));
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK(not voidNode.state().errored());
	// try to set data and then pull
	BOOST_REQUIRE_NO_THROW(voidNode.port(0).set<float>(2.0f));
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 2.0f);
	BOOST_CHECK(not voidNode.state().errored());
	// disconnect and test the value + throw on pull
	BOOST_REQUIRE_NO_THROW(voidNode.port(1).disconnect(floatNode.port(0)));
	BOOST_CHECK_THROW(voidNode.port(1).get<float>(), std::runtime_error);
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 2.0f);
	BOOST_CHECK(/*not*/ voidNode.state().errored());


	// connect int, try to pull
	BOOST_REQUIRE_NO_THROW(voidNode.port(1).connect(intNode.port(0)));
	BOOST_CHECK_EQUAL(voidNode.port(1).get<int>(), 2);
	BOOST_CHECK_EQUAL(intNode.port(1).get<int>(), 2);
	BOOST_CHECK(not voidNode.state().errored());
	// try to set data and then pull
	BOOST_REQUIRE_NO_THROW(voidNode.port(0).set<float>(2));
	BOOST_CHECK_EQUAL(voidNode.port(1).get<int>(), 2);
	BOOST_CHECK_EQUAL(intNode.port(1).get<int>(), 2);
	BOOST_CHECK(not voidNode.state().errored());
	// disconnect and test the default value + throw on pull
	BOOST_REQUIRE_NO_THROW(voidNode.port(1).disconnect(intNode.port(0)));
	BOOST_CHECK_THROW(voidNode.port(1).get<int>(), std::runtime_error);
	BOOST_CHECK_EQUAL(intNode.port(1).get<int>(), 2);
	BOOST_CHECK(/*not*/ voidNode.state().errored());


	// connect unsigned, try to pull (and get an exception)
	BOOST_REQUIRE_NO_THROW(voidNode.port(1).connect(unsignedNode.port(0)));
	BOOST_CHECK_EQUAL(voidNode.port(1).get<unsigned>(), 0);
	BOOST_CHECK_EQUAL(unsignedNode.port(1).get<unsigned>(), 0);
	BOOST_CHECK(voidNode.state().errored());
	// try to set data and then pull
	BOOST_REQUIRE_NO_THROW(voidNode.port(0).set<float>(2));
	BOOST_CHECK_EQUAL(voidNode.port(1).get<unsigned>(), 0u);
	BOOST_CHECK_EQUAL(unsignedNode.port(1).get<unsigned>(), 0u);
	BOOST_CHECK(voidNode.state().errored());
	// disconnect and test the default value + throw on pull
	BOOST_REQUIRE_NO_THROW(voidNode.port(1).disconnect(unsignedNode.port(0)));
	BOOST_CHECK_THROW(voidNode.port(1).get<unsigned>(), std::runtime_error);
	BOOST_CHECK_EQUAL(unsignedNode.port(1).get<unsigned>(), 0u);
	BOOST_CHECK(voidNode.state().errored());
}


/////////////////


// untyped INPUT port evaluation:
//   - simple - compute() takes a float
BOOST_AUTO_TEST_CASE(void_simple_in) {
	Graph g;

	// make a simple assignment node handle, float "float" to "untyped"
	const HandleRegistrar& voidHandle = typedNode<void, float>(Assign<void, float>::compute);
	const HandleRegistrar& floatHandle = typedNode<float, float>(Assign<float, float>::compute);

	NodeBase& voidNode = g.nodes().add(voidHandle, "void_node");
	NodeBase& floatNode = g.nodes().add(floatHandle, "float_node");

	BOOST_REQUIRE(voidNode.portCount() == 2);
	BOOST_REQUIRE(voidNode.port(0).type() == unmangledTypeId<void>());
	BOOST_REQUIRE(voidNode.port(1).type() == unmangledTypeId<float>());

	BOOST_REQUIRE(floatNode.portCount() == 2);
	BOOST_REQUIRE(floatNode.port(0).type() == unmangledTypeId<float>());
	BOOST_REQUIRE(floatNode.port(1).type() == unmangledTypeId<float>());

	// the void input doesn't have a type yet as its not connected - reading should throw
	BOOST_CHECK_THROW(voidNode.port(0).get<float>(), std::runtime_error);
	BOOST_CHECK(not voidNode.state().errored());

	// pulling on output will work, but will error because the void input has no type
	BOOST_CHECK_NO_THROW(voidNode.port(1).get<float>());
	BOOST_CHECK(voidNode.state().errored());

	// connect the two nodes together, to "assign a type" to the void node output
	BOOST_REQUIRE_NO_THROW(floatNode.port(1).connect(voidNode.port(0)));

	// the void input now has a type and can be read
	BOOST_CHECK_NO_THROW(voidNode.port(0).get<float>());
	BOOST_CHECK_EQUAL(voidNode.port(0).get<float>(), 0.0f);
	// but no evaluation was triggered, so the state of the node stays
	BOOST_CHECK(voidNode.state().errored());

	// pull on the void node output (this should now work and not error)
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK(not voidNode.state().errored());

	// assign a new value to the float node input
	BOOST_REQUIRE_NO_THROW(floatNode.port(0).set(5.0f));
	BOOST_CHECK(not voidNode.state().errored());
	// // check dirty propagation
	BOOST_CHECK(voidNode.port(0).isDirty());
	BOOST_CHECK(voidNode.port(1).isDirty());
	BOOST_CHECK(floatNode.port(1).isDirty());
	BOOST_CHECK(not voidNode.state().errored());

	// and check the output on both nodes (i.e., via pull, both directly and indirectly)
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 5.0f);
	BOOST_CHECK(not voidNode.state().errored());
	// and check nothing stayed dirty after evaluation
	BOOST_CHECK(not voidNode.port(0).isDirty());
	BOOST_CHECK(not voidNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.port(0).isDirty());
	BOOST_CHECK(not floatNode.port(1).isDirty());
	BOOST_CHECK(not voidNode.state().errored());
}


// 1. untyped INPUT port evaluation on error:
// 	 throw an exception during evaluation
// 2. untyped INPUT incorrect type request:
//   get() incorrect datatype (should throw an exception, but pulling on the connected out should work)
namespace {

void untyped_input_handle_error_test(const HandleRegistrar& voidHandle) {
	Graph g;

	const HandleRegistrar& floatHandle = typedNode<float, float>(Assign<float, float>::compute);

	NodeBase& voidNode = g.nodes().add(voidHandle, "void_node");
	NodeBase& floatNode = g.nodes().add(floatHandle, "float_node");

	BOOST_REQUIRE(voidNode.portCount() == 2);
	BOOST_REQUIRE(voidNode.port(0).type() == unmangledTypeId<void>());
	BOOST_REQUIRE(voidNode.port(1).type() == unmangledTypeId<float>());

	BOOST_REQUIRE(floatNode.portCount() == 2);
	BOOST_REQUIRE(floatNode.port(0).type() == unmangledTypeId<float>());
	BOOST_REQUIRE(floatNode.port(1).type() == unmangledTypeId<float>());

	// the void intpu doesn't have a type yet as its not connected - reading should error, but not throw
	BOOST_CHECK_NO_THROW(voidNode.port(1).get<float>());
	BOOST_CHECK(voidNode.state().errored());

	// connect the two nodes together, to "assign a type" to the void node intpu
	BOOST_REQUIRE_NO_THROW(floatNode.port(1).connect(voidNode.port(0)));

	// the void input now has a type and can be read
	BOOST_CHECK_NO_THROW(voidNode.port(0).get<float>());
	// pull on the void node output (this should work, but return default-constructed float)
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);

	// but compute() thrown an exception, so the state of the node should be invalid
	BOOST_CHECK(voidNode.state().errored());
	BOOST_CHECK(not floatNode.state().errored());

	// assign a new value to the float node input
	BOOST_REQUIRE_NO_THROW(floatNode.port(0).set(5.0f));
	// check dirty propagation
	BOOST_CHECK(voidNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.port(0).isDirty());
	BOOST_CHECK(floatNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.state().errored());
	BOOST_CHECK(voidNode.state().errored());

	// and check the output on both nodes (i.e., via pull, both directly and indirectly)
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK(not floatNode.state().errored());
	BOOST_CHECK(voidNode.state().errored());
	// and check nothing stayed dirty after evaluation
	BOOST_CHECK(not voidNode.port(0).isDirty());
	BOOST_CHECK(not voidNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.port(0).isDirty());
	BOOST_CHECK(not floatNode.port(1).isDirty());
	BOOST_CHECK(not floatNode.state().errored());
	BOOST_CHECK(voidNode.state().errored());
}

}

// 1. untyped INPUT port evaluation on error:
// 	 throw an exception during evaluation
BOOST_AUTO_TEST_CASE(void_error_in) {
	// make a simple assignment node handle, "untyped" to "float"
	const HandleRegistrar& voidHandle = typedNode<void, float>(
		[](dependency_graph::Values& val, const InAttr<void>& in, const OutAttr<float>& out) {
			throw std::runtime_error("error");
		}
	);

	untyped_input_handle_error_test(voidHandle);
}

// 2. untyped INPUT incorrect type request:
//   get() incorrect datatype (should throw an exception, but pulling on the connected out should work)
BOOST_AUTO_TEST_CASE(void_error_assignment_in) {
	// make a simple assignment node handle, float "float" to "untyped"
	const HandleRegistrar& voidHandle = typedNode<void, float>(
		[](dependency_graph::Values& val, const InAttr<void>& in, const OutAttr<float>& out) {
			// assigns an integer - assuming connected node is float, this should throw an exception
			val.set(out, (float)val.get<int>(in));
		}
	);

	untyped_input_handle_error_test(voidHandle);
}

// untyped INPUT port evaluation:
//   - more complex - based on the input type, do evaluation with the right type, always return a float
BOOST_AUTO_TEST_CASE(void_complex_in) {
	Graph g;

	// apart from simple assignment nodes, make one node that can handle both
	//   float and int, depending on what is connected to it (and throws an exception
	//   on an unknown type)
	const MetadataHandle voidHandle = typedNode<void, float>(
		[](dependency_graph::Values& val, const InAttr<void>& in, const OutAttr<float>& out) {
			// based on type, do the right thing or throw
			if(val.is<float>(in))
				val.set(out, val.get<float>(in));
			else if(val.is<int>(in))
				val.set(out, (float)val.get<int>(in));
			else
				throw std::runtime_error("unknown");
		}
	);
	const MetadataHandle floatHandle = typedNode<float, float>(Assign<float, float>::compute);
	const MetadataHandle intHandle = typedNode<int, int>(Assign<int, int, int>::compute);
	const MetadataHandle unsignedHandle = typedNode<unsigned, unsigned>(Assign<unsigned, unsigned, unsigned>::compute);

	NodeBase& voidNode = g.nodes().add(voidHandle, "void_node");
	NodeBase& floatNode = g.nodes().add(floatHandle, "float_node");
	NodeBase& intNode = g.nodes().add(intHandle, "int_node");
	NodeBase& unsignedNode = g.nodes().add(unsignedHandle, "unsigned_node");

	// first, lets try the void node on its own, with no connections
	BOOST_CHECK_THROW(voidNode.port(0).get<float>(), std::runtime_error);

	// connect float, try to pull
	BOOST_REQUIRE_NO_THROW(floatNode.port(1).connect(voidNode.port(0)));
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK(not voidNode.state().errored());
	// try to set data and then pull
	BOOST_REQUIRE_NO_THROW(floatNode.port(0).set<float>(2.0f));
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 2.0f);
	BOOST_CHECK(not voidNode.state().errored());
	// disconnect and test the value + throw on pull
	BOOST_REQUIRE_NO_THROW(floatNode.port(1).disconnect(voidNode.port(0)));
	BOOST_CHECK_NO_THROW(voidNode.port(1).get<float>());
	BOOST_CHECK_EQUAL(floatNode.port(1).get<float>(), 2.0f);
	BOOST_CHECK(voidNode.state().errored());  // trying to recompute output from an unconnected input


	// connect int, try to pull
	BOOST_REQUIRE_NO_THROW(intNode.port(1).connect(voidNode.port(0)));
	BOOST_CHECK_EQUAL(intNode.port(1).get<int>(), 0);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK(not voidNode.state().errored());
	// try to set data and then pull
	BOOST_REQUIRE_NO_THROW(intNode.port(0).set<int>(2));
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(intNode.port(1).get<int>(), 2);
	BOOST_CHECK(not voidNode.state().errored());
	// disconnect and test the default value + throw on pull
	BOOST_REQUIRE_NO_THROW(intNode.port(1).disconnect(voidNode.port(0)));
	BOOST_CHECK_THROW(voidNode.port(0).get<int>(), std::runtime_error);
	BOOST_CHECK_EQUAL(intNode.port(1).get<int>(), 2);
	BOOST_CHECK(not voidNode.state().errored());


	// connect unsigned, try to pull
	BOOST_REQUIRE_NO_THROW(unsignedNode.port(1).connect(voidNode.port(0)));
	BOOST_CHECK_EQUAL(voidNode.port(0).get<unsigned>(), 0);
	BOOST_CHECK_EQUAL(unsignedNode.port(1).get<unsigned>(), 0);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK(voidNode.state().errored());
	// try to set data and then pull
	BOOST_REQUIRE_NO_THROW(unsignedNode.port(0).set<unsigned>(2));
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(unsignedNode.port(1).get<unsigned>(), 2u);
	BOOST_CHECK(voidNode.state().errored());
	// disconnect and test the default value + throw on pull
	BOOST_REQUIRE_NO_THROW(unsignedNode.port(1).disconnect(voidNode.port(0)));
	BOOST_CHECK_THROW(voidNode.port(0).get<unsigned>(), std::runtime_error);
	BOOST_CHECK_EQUAL(unsignedNode.port(1).get<unsigned>(), 2u);
	BOOST_CHECK_EQUAL(voidNode.port(1).get<float>(), 0.0f);
	BOOST_CHECK(voidNode.state().errored());
}

// TODO
// test error with a void pointer - reset() of an out port with void type
//   after compute() throws an exception

