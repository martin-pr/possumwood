#include <boost/test/unit_test.hpp>

#include "graph.h"
#include "attr.inl"
#include "datablock.inl"
#include "metadata.inl"
#include "node.inl"
#include "common.h"

BOOST_AUTO_TEST_CASE(arithmetic) {

	//////////////////
	// addition node

	// metadata for a simple addition node
	const Metadata& addition = additionNode();
	// and for a simple multiplication node
	const Metadata& multiplication = multiplicationNode();

	/////////////////////////////
	// build a simple graph for (a + b) * c + (a + b) * d

	Graph g;

	Node& add1 = g.nodes().add(addition, "add_1");
	Node& mult1 = g.nodes().add(multiplication, "mult_1");
	Node& mult2 = g.nodes().add(multiplication, "mult_2");
	Node& add2 = g.nodes().add(addition, "add_2");

	// a valid connection
	BOOST_CHECK_NO_THROW(add1.port(2).connect(mult1.port(1)));
	BOOST_CHECK_NO_THROW(add1.port(2).connect(mult2.port(1)));
	BOOST_CHECK_NO_THROW(mult1.port(2).connect(add2.port(0)));
	BOOST_CHECK_NO_THROW(mult2.port(2).connect(add2.port(1)));

	// invalid connections
	BOOST_CHECK_THROW(add1.port(1).connect(mult1.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(2).connect(mult1.port(2)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(1).connect(mult1.port(2)), std::runtime_error);

	/////////////////////////////
	// compute (2 + 3) * 4 + (2 + 3) * 5 = 20 + 25 = 45

	// set input values
	BOOST_CHECK_NO_THROW(add1.port(0).set(2.0f));
	BOOST_CHECK_NO_THROW(add1.port(1).set(3.0f));
	BOOST_CHECK_NO_THROW(mult1.port(0).set(4.0f));
	BOOST_CHECK_NO_THROW(mult2.port(0).set(5.0f));

	// evaluate the whole graph and test output
	BOOST_CHECK_EQUAL(add2.port(2).get<float>(), 45.0f);

	// and test partial outputs
	BOOST_CHECK_EQUAL(add1.port(2).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(mult1.port(2).get<float>(), 20.0f);
	BOOST_CHECK_EQUAL(mult2.port(2).get<float>(), 25.0f);
}
