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
	addition.addInfluence(additionInput1, additionOutput);
	addition.addInfluence(additionInput2, additionOutput);

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

	Node& add = g.nodes().add(addition, "add");
	Node& mult = g.nodes().add(multiplication, "mult");

	g.connections().add(add.port(2), mult.port(1));
}
