#include <boost/test/unit_test.hpp>

#include "graph.h"
#include "common.h"

namespace {
	bool checkConnections(const Graph& g, std::vector<std::pair<const Node::Port*, const Node::Port*>> connections) {
		bool result = true;

		for(auto c : g.connections()) {
			std::pair<const Node::Port*, const Node::Port*> key(&c.first, &c.second);

			auto it = std::find(connections.begin(), connections.end(), key);
			if(it == connections.end())
				result = false;
			else
				connections.erase(it);
		}

		return connections.empty() && result;
	}
}

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

	//////////
	// adding

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

	// add another connection
	BOOST_CHECK_NO_THROW(add1.port(2).connect(mult2.port(1)));

	BOOST_REQUIRE_EQUAL(g.connections().size(), 2u);
	BOOST_REQUIRE(g.connections().begin() != g.connections().end());
	{
		auto it = g.connections().begin();
		++it; ++it;
		BOOST_REQUIRE(it == g.connections().end());
	}
	BOOST_REQUIRE(checkConnections(g, {{&add1.port(2), &mult1.port(1)}, {&add1.port(2), &mult2.port(1)}}));

	// add another two connections to complete a graph
	BOOST_CHECK_NO_THROW(mult1.port(2).connect(add2.port(0)));
	BOOST_CHECK_NO_THROW(mult2.port(2).connect(add2.port(1)));

	BOOST_REQUIRE_EQUAL(g.connections().size(), 4u);
	BOOST_REQUIRE(g.connections().begin() != g.connections().end());
	{
		auto it = g.connections().begin();
		++it; ++it; ++it; ++it;
		BOOST_REQUIRE(it == g.connections().end());
	}
	BOOST_REQUIRE(checkConnections(g, {
		{&add1.port(2), &mult1.port(1)},
		{&add1.port(2), &mult2.port(1)},
		{&mult1.port(2), &add2.port(0)},
		{&mult2.port(2), &add2.port(1)}
	}));

	// try to add invalid connections
	BOOST_CHECK_THROW(add1.port(1).connect(mult1.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(2).connect(mult1.port(2)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(1).connect(mult1.port(2)), std::runtime_error);

	/////////
	// removing

	// try to remove a connection
	BOOST_CHECK_NO_THROW(add1.port(2).disconnect(mult1.port(1)));

	BOOST_REQUIRE_EQUAL(g.connections().size(), 3u);
	BOOST_REQUIRE(checkConnections(g, {
		{&add1.port(2), &mult2.port(1)},
		{&mult1.port(2), &add2.port(0)},
		{&mult2.port(2), &add2.port(1)}
	}));

	// try to remove the same connection again
	BOOST_CHECK_THROW(add1.port(2).disconnect(mult1.port(1)), std::runtime_error);

	// try to remove invalid connections
	BOOST_CHECK_THROW(add1.port(1).disconnect(mult1.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(2).disconnect(mult1.port(2)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(1).disconnect(mult1.port(2)), std::runtime_error);
}
