#include <boost/test/unit_test.hpp>

#include "graph.h"
#include "attr.h"
#include "datablock.h"
#include "metadata.h"

BOOST_AUTO_TEST_CASE(arithmetic) {

	//////////////////
	// addition node

	InAttr<float> additionInput1, additionInput2;
	OutAttr<float> additionOutput;
	std::function<void(Datablock&)> additionCompute = [&](Datablock & data) {
		const float a = data.get(additionInput1);
		const float b = data.get(additionInput2);

		data.set(additionOutput, a + b);
	};

	Metadata addition("addition");

	addition.addAttribute(additionInput1, "input_1");
	addition.addAttribute(additionInput2, "input_2");
	addition.addAttribute(additionOutput, "output");

	addition.addInfluence(additionInput1, additionOutput);
	addition.addInfluence(additionInput2, additionOutput);

	addition.setCompute(additionCompute);

	////////////////////////
	// multiplication node

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
	// build a simple graph for a + b * c

	Graph g;

	Node& add = g.nodes().add(addition, "add");
	Node& mult = g.nodes().add(multiplication, "mult");

	g.connections().add(add.port("output"), mult.port("input_2"));
}
