#include <boost/test/unit_test.hpp>

#include "graph.h"
#include "common.h"

using namespace dependency_graph;

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

	unsigned s_connectionCount = 0;
}

BOOST_AUTO_TEST_CASE(graph_connections) {
	Graph g;

	//////
	// connect callbacks
	g.onConnect([&](Node::Port&, Node::Port&) { ++s_connectionCount; });
	g.onDisconnect([&](Node::Port&, Node::Port&) { --s_connectionCount; });

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

	BOOST_CHECK_EQUAL(s_connectionCount, 0u);

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

	BOOST_CHECK_EQUAL(s_connectionCount, 1u);

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

	BOOST_CHECK_EQUAL(s_connectionCount, 2u);

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

	BOOST_CHECK_EQUAL(s_connectionCount, 4u);

	// try to add invalid connections
	BOOST_CHECK_THROW(add1.port(1).connect(mult1.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(2).connect(mult1.port(2)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(1).connect(mult1.port(2)), std::runtime_error);

	BOOST_CHECK_EQUAL(s_connectionCount, 4u);

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

	BOOST_CHECK_EQUAL(s_connectionCount, 3u);

	// try to remove the same connection again
	BOOST_CHECK_THROW(add1.port(2).disconnect(mult1.port(1)), std::runtime_error);

	BOOST_CHECK_EQUAL(s_connectionCount, 3u);

	// try to remove invalid connections
	BOOST_CHECK_THROW(add1.port(1).disconnect(mult1.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(2).disconnect(mult1.port(2)), std::runtime_error);
	BOOST_CHECK_THROW(add1.port(1).disconnect(mult1.port(2)), std::runtime_error);

	BOOST_CHECK_EQUAL(s_connectionCount, 3u);

	// remove mult2 node, which should also remove relevant connections
	auto it = std::find_if(g.nodes().begin(), g.nodes().end(), [&](const Node& n) {
		return n.name() == "mult_2";
	});
	BOOST_REQUIRE(it != g.nodes().end());
	g.nodes().erase(it);

	// now all two connections of the mult2 node should be removed as well
	BOOST_REQUIRE_EQUAL(g.connections().size(), 1u);
	BOOST_REQUIRE(checkConnections(g, {
		{&mult1.port(2), &add2.port(0)}
	}));

	BOOST_CHECK_EQUAL(s_connectionCount, 1u);

	// and clear() shold remove the rest
	g.clear();
	BOOST_REQUIRE_EQUAL(g.connections().size(), 0u);
	BOOST_REQUIRE(g.connections().empty());
	BOOST_REQUIRE(g.nodes().empty());

	BOOST_CHECK_EQUAL(s_connectionCount, 0u);
}
