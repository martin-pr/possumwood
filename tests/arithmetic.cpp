#include <boost/test/unit_test.hpp>

#include "graph.h"
#include "attr.inl"
#include "datablock.inl"
#include "metadata.inl"
#include "node.inl"

BOOST_AUTO_TEST_CASE(arithmetic) {

	//////////////////
	// addition node

	// base initialisation
	Metadata addition("addition");
	BOOST_REQUIRE_EQUAL(addition.type(), "addition");

	// create attributes
	InAttr<float> additionInput1, additionInput2;
	OutAttr<float> additionOutput;
	BOOST_CHECK(not additionInput1.isValid());
	BOOST_CHECK(not additionInput2.isValid());
	BOOST_CHECK(not additionOutput.isValid());

	// add attributes to the Metadata instance
	addition.addAttribute(additionInput1, "input_1");
	addition.addAttribute(additionInput2, "input_2");
	addition.addAttribute(additionOutput, "output");

	BOOST_REQUIRE_EQUAL(addition.attributeCount(), 3u);
	BOOST_CHECK_EQUAL(&addition.attr(0), &additionInput1);
	BOOST_CHECK_EQUAL(&addition.attr(1), &additionInput2);
	BOOST_CHECK_EQUAL(&addition.attr(2), &additionOutput);

	// setup influences
	BOOST_CHECK_NO_THROW(addition.addInfluence(additionInput1, additionOutput));
	BOOST_CHECK_NO_THROW(addition.addInfluence(additionInput2, additionOutput));

	std::vector<std::reference_wrapper<const Attr>> influences;

	BOOST_CHECK_NO_THROW(influences = addition.influences(additionInput1));
	BOOST_REQUIRE_EQUAL(influences.size(), 1u);
	BOOST_CHECK_EQUAL(&(influences.begin()->get()), &additionOutput);

	BOOST_CHECK_NO_THROW(influences = addition.influencedBy(additionOutput));
	BOOST_REQUIRE_EQUAL(influences.size(), 2u);
	BOOST_CHECK_EQUAL(&(influences[0].get()), &additionInput1);
	BOOST_CHECK_EQUAL(&(influences[1].get()), &additionInput2);

	std::function<void(Datablock&)> additionCompute = [&](Datablock & data) {
		const float a = data.get(additionInput1);
		const float b = data.get(additionInput2);

		data.set(additionOutput, a + b);
	};
	addition.setCompute(additionCompute);

	////////////////////////
	// multiplication node

	// no need to test again what was tested above

	InAttr<float> multiplicationInput1, multiplicationInput2;
	OutAttr<float> multiplicationOutput;
	std::function<void(Datablock&)> multiplicationCompute = [&](Datablock & data) {
		const float a = data.get(multiplicationInput1);
		const float b = data.get(multiplicationInput2);

		data.set(multiplicationOutput, a * b);
	};

	Metadata multiplication("multiplication");

	multiplication.addAttribute(multiplicationInput1, "input_1");
	multiplication.addAttribute(multiplicationInput2, "input_2");
	multiplication.addAttribute(multiplicationOutput, "output");

	multiplication.addInfluence(multiplicationInput1, multiplicationOutput);
	multiplication.addInfluence(multiplicationInput2, multiplicationOutput);

	multiplication.setCompute(multiplicationCompute);

	/////////////////////////////
	// build a simple graph for (a + b) * c

	Graph g;

	Node& add1 = g.nodes().add(addition, "add_1");
	Node& mult1 = g.nodes().add(multiplication, "mult_1");
	Node& mult2 = g.nodes().add(multiplication, "mult_2");
	Node& add2 = g.nodes().add(multiplication, "add_2");

	// a valid connection
	BOOST_CHECK_NO_THROW(g.connections().add(add1.port(2), mult1.port(1)));
	BOOST_CHECK_NO_THROW(g.connections().add(add1.port(2), mult2.port(1)));
	BOOST_CHECK_NO_THROW(g.connections().add(mult1.port(2), add2.port(0)));
	BOOST_CHECK_NO_THROW(g.connections().add(mult2.port(2), add2.port(1)));

	// // invalid connections
	BOOST_CHECK_THROW(g.connections().add(add1.port(1), mult1.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(g.connections().add(add1.port(2), mult1.port(2)), std::runtime_error);
	BOOST_CHECK_THROW(g.connections().add(add1.port(1), mult1.port(2)), std::runtime_error);

	/////////////////////////////
	// compute (2 + 3) * 4 + (2 + 3) * 5 = 20

	add1.port(0).set(2.0f);
	add1.port(1).set(3.0f);
	mult1.port(0).set(4.0f);
	mult2.port(0).set(5.0f);

	BOOST_CHECK_EQUAL(add2.port(2).get<float>(), 20.0f);
}
