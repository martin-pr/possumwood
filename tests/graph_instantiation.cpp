#include <boost/test/unit_test.hpp>

#include "graph.h"
#include "common.h"

namespace {

unsigned s_nodeCount = 0;

}

BOOST_AUTO_TEST_CASE(graph_instantiation) {
	Graph g;

	//////
	// connect callbacks
	g.onAddNode([&](Node&) { ++s_nodeCount; });
	g.onRemoveNode([&](Node&) { --s_nodeCount; });

	/////
	// empty graph
	BOOST_REQUIRE(g.empty());
	BOOST_REQUIRE(g.nodes().empty());
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 0u);
	BOOST_REQUIRE(g.nodes().begin() == g.nodes().end());

	/////
	// add one node

	BOOST_REQUIRE_NO_THROW(g.nodes().add(additionNode(), "add_1"));

	BOOST_REQUIRE(not g.empty());
	BOOST_REQUIRE(not g.nodes().empty());
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 1u);
	BOOST_REQUIRE(g.nodes().begin() != g.nodes().end());
	BOOST_REQUIRE(g.nodes().begin() + 1 == g.nodes().end());

	BOOST_REQUIRE_EQUAL(&(g.nodes()[0].metadata()), &(additionNode()));
	BOOST_REQUIRE_EQUAL(g.nodes()[0].name(), "add_1");

	BOOST_REQUIRE_EQUAL(&(g.nodes().begin()->metadata()), &(additionNode()));
	BOOST_REQUIRE_EQUAL(g.nodes().begin()->name(), "add_1");

	BOOST_CHECK_EQUAL(s_nodeCount, 1u);

	/////
	// add two more nodes

	BOOST_REQUIRE_NO_THROW(g.nodes().add(multiplicationNode(), "mult_1"));
	BOOST_REQUIRE_NO_THROW(g.nodes().add(additionNode(), "add_2"));

	BOOST_REQUIRE(not g.empty());
	BOOST_REQUIRE(not g.nodes().empty());
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 3u);
	BOOST_REQUIRE(g.nodes().begin() != g.nodes().end());

	for(size_t a = 0; a < 3; ++a)
		BOOST_REQUIRE_EQUAL(&(g.nodes()[a]), &(*(g.nodes().begin() + a)));

	BOOST_REQUIRE_EQUAL(&(g.nodes()[0].metadata()), &(additionNode()));
	BOOST_REQUIRE_EQUAL(g.nodes()[0].name(), "add_1");
	BOOST_REQUIRE_EQUAL(&(g.nodes()[1].metadata()), &(multiplicationNode()));
	BOOST_REQUIRE_EQUAL(g.nodes()[1].name(), "mult_1");
	BOOST_REQUIRE_EQUAL(&(g.nodes()[2].metadata()), &(additionNode()));
	BOOST_REQUIRE_EQUAL(g.nodes()[2].name(), "add_2");

	BOOST_CHECK_EQUAL(s_nodeCount, 3u);

	/////
	// remove one node (mult_1)

	BOOST_REQUIRE_NO_THROW(g.nodes().erase(g.nodes().begin() + 1));

	BOOST_REQUIRE(not g.empty());
	BOOST_REQUIRE(not g.nodes().empty());
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 2u);
	BOOST_REQUIRE(g.nodes().begin() != g.nodes().end());
	BOOST_REQUIRE(g.nodes().begin() + 2 == g.nodes().end());

	for(size_t a = 0; a < 2; ++a)
		BOOST_REQUIRE_EQUAL(&(g.nodes()[a]), &(*(g.nodes().begin() + a)));

	BOOST_REQUIRE_EQUAL(&(g.nodes()[0].metadata()), &(additionNode()));
	BOOST_REQUIRE_EQUAL(g.nodes()[0].name(), "add_1");
	BOOST_REQUIRE_EQUAL(&(g.nodes()[1].metadata()), &(additionNode()));
	BOOST_REQUIRE_EQUAL(g.nodes()[1].name(), "add_2");

	BOOST_CHECK_EQUAL(s_nodeCount, 2u);

	/////
	// clear the graph

	BOOST_REQUIRE_NO_THROW(g.clear());

	BOOST_REQUIRE(g.empty());
	BOOST_REQUIRE(g.nodes().empty());
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 0u);
	BOOST_REQUIRE(g.nodes().begin() == g.nodes().end());

	BOOST_CHECK_EQUAL(s_nodeCount, 0u);
}
