#include <boost/test/unit_test.hpp>

#include <dependency_graph/graph.h>
#include <dependency_graph/nodes.inl>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/datablock.inl>
#include "common.h"

using namespace dependency_graph;

namespace {
	bool checkConnections(const Graph& g, std::vector<std::pair<const Port*, const Port*>> connections) {
		bool result = true;

		for(auto c : g.connections()) {
			std::pair<const Port*, const Port*> key(&c.first, &c.second);

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
	g.onConnect([&](Port&, Port&) { ++s_connectionCount; });
	g.onDisconnect([&](Port&, Port&) { --s_connectionCount; });

	// instantiate nodes from arithmetic.cpp
	const MetadataHandle& addition = additionNode();
	const MetadataHandle& multiplication = multiplicationNode();

	NodeBase& add1 = g.nodes().add(addition, "add_1");
	NodeBase& mult1 = g.nodes().add(multiplication, "mult_1");
	NodeBase& mult2 = g.nodes().add(multiplication, "mult_2");
	NodeBase& add2 = g.nodes().add(addition, "add_2");

	// 4 nodes and no connections
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 4u);
	BOOST_REQUIRE_EQUAL(g.connections().size(), 0u);
	BOOST_REQUIRE(g.connections().begin() == g.connections().end());

	BOOST_CHECK_EQUAL(s_connectionCount, 0u);

	// set some values of them, to test the evaluation results
	BOOST_REQUIRE_NO_THROW(add1.port(0).set(1.0f));
	BOOST_REQUIRE_NO_THROW(add1.port(1).set(2.0f));
	BOOST_CHECK_EQUAL(add1.port(2).get<float>(), 3.0f);

	BOOST_REQUIRE_NO_THROW(mult1.port(0).set(4.0f));
	BOOST_REQUIRE_NO_THROW(mult1.port(1).set(5.0f));
	BOOST_CHECK_EQUAL(mult1.port(2).get<float>(), 20.0f);

	BOOST_REQUIRE_NO_THROW(add2.port(0).set(6.0f));
	BOOST_REQUIRE_NO_THROW(add2.port(1).set(7.0f));
	BOOST_CHECK_EQUAL(add2.port(2).get<float>(), 13.0f);

	BOOST_REQUIRE_NO_THROW(mult2.port(0).set(8.0f));
	BOOST_REQUIRE_NO_THROW(mult2.port(1).set(9.0f));
	BOOST_CHECK_EQUAL(mult2.port(2).get<float>(), 72.0f);

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

	// test values of a single connection (PULLS on the output)
	BOOST_CHECK_EQUAL(add1.port(2).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(mult1.port(1).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(mult1.port(2).get<float>(), 12.0f);

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

	// try to connect a single node's output to its input
	BOOST_CHECK_THROW(add1.port(2).connect(add1.port(1)), std::runtime_error);
	// and try to connect the end of the chain to its beginning
	BOOST_CHECK_THROW(mult2.port(2).connect(add1.port(1)), std::runtime_error);

	BOOST_CHECK_EQUAL(s_connectionCount, 4u);

	// test the final computation result (will pull recursively on everything)
	BOOST_CHECK_EQUAL(add2.port(2).get<float>(), 36.0f);


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
	auto it = std::find_if(g.nodes().begin(), g.nodes().end(), [&](const NodeBase& n) {
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

	// and clear() should remove the rest
	g.clear();
	BOOST_REQUIRE_EQUAL(g.connections().size(), 0u);
	BOOST_REQUIRE(g.connections().empty());
	BOOST_REQUIRE(g.nodes().empty());

	BOOST_CHECK_EQUAL(s_connectionCount, 0u);
}
