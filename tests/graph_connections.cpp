#include <boost/test/unit_test.hpp>

#include "graph.h"
#include "common.h"

BOOST_AUTO_TEST_CASE(graph_connections) {
	Graph g;

	// instantiate nodes from arithmetic.cpp
	const Metadata& addition = additionNode();
	const Metadata& multiplication = multiplicationNode();

	Node& add1 = g.nodes().add(addition, "add_1");
	Node& mult1 = g.nodes().add(multiplication, "mult_1");
	Node& mult2 = g.nodes().add(multiplication, "mult_2");
	Node& add2 = g.nodes().add(addition, "add_2");

	// 4 nodes and no connections
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 4u);
	BOOST_REQUIRE_EQUAL(g.connections().size(), 0u);
	BOOST_REQUIRE(g.connections().begin() == g.connections().end());

	// add a single connection
	BOOST_CHECK_NO_THROW(add1.port(2).connect(mult1.port(1)));

	BOOST_REQUIRE_EQUAL(g.connections().size(), 1u);
	BOOST_REQUIRE(g.connections().begin() != g.connections().end());
	{
		auto it = g.connections().begin();
		++it;
		BOOST_REQUIRE(it == g.connections().end());
	}

	BOOST_CHECK_EQUAL(&(g.connections().begin()->first), &add1.port(2));
	BOOST_CHECK_EQUAL(&(g.connections().begin()->second), &mult1.port(1));
}
