#include <boost/test/unit_test.hpp>

#include <dependency_graph/graph.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes.inl>

#include "common.h"

using namespace dependency_graph;

namespace {

unsigned s_nodeCount = 0;

}

namespace dependency_graph {

// static std::ostream& operator << (std::ostream& out, dependency_graph::MetadataHandle h) {
// 	std::cout << "Metadata for " << h.metadata().type();

// 	return out;
// }

}

bool testIterations(Nodes::const_iterator i1, Nodes::const_iterator i2, std::size_t count) {
	std::size_t result = 0;
	while(i1 != i2) {
		++i1;
		++result;
	}
	return result == count;
}

BOOST_AUTO_TEST_CASE(graph_instantiation) {
	Graph g;

	//////
	// connect callbacks
	g.onAddNode([&](NodeBase&) { ++s_nodeCount; });
	g.onRemoveNode([&](NodeBase&) { --s_nodeCount; });

	/////
	// empty graph
	BOOST_REQUIRE(g.empty());
	BOOST_REQUIRE(g.nodes().empty());
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 0u);
	BOOST_REQUIRE(g.nodes().begin() == g.nodes().end());

	/////
	// add one node

	std::vector<UniqueId> ids;

	BOOST_REQUIRE_NO_THROW(ids.push_back(g.nodes().add(additionNode(), "add_1").index()));

	BOOST_REQUIRE(not g.empty());
	BOOST_REQUIRE(not g.nodes().empty());
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 1u);
	BOOST_REQUIRE(g.nodes().begin() != g.nodes().end());
	BOOST_REQUIRE(testIterations(g.nodes().begin(), g.nodes().end(), 1));

	BOOST_REQUIRE_EQUAL(&g.nodes()[ids[0]].metadata().metadata(), &additionNode().metadata());
	BOOST_REQUIRE_EQUAL(g.nodes()[ids[0]].name(), "add_1");

	BOOST_REQUIRE_EQUAL(&g.nodes().begin()->metadata().metadata(), &additionNode().metadata());
	BOOST_REQUIRE_EQUAL(g.nodes().begin()->name(), "add_1");

	BOOST_CHECK_EQUAL(s_nodeCount, 1u);

	/////
	// add two more nodes

	BOOST_REQUIRE_NO_THROW(ids.push_back(g.nodes().add(multiplicationNode(), "mult_1").index()));
	BOOST_REQUIRE_NO_THROW(ids.push_back(g.nodes().add(additionNode(), "add_2").index()));

	BOOST_REQUIRE(not g.empty());
	BOOST_REQUIRE(not g.nodes().empty());
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 3u);
	BOOST_REQUIRE(g.nodes().begin() != g.nodes().end());

	{
		auto it = g.nodes().begin();
		for(size_t a = 0; a < 3; ++a)
			BOOST_REQUIRE_EQUAL(&(g.nodes()[ids[a]]), &(*(it++)));
	}

	BOOST_REQUIRE_EQUAL(&g.nodes()[ids[0]].metadata().metadata(), &additionNode().metadata());
	BOOST_REQUIRE_EQUAL(g.nodes()[ids[0]].name(), "add_1");
	BOOST_REQUIRE_EQUAL(&g.nodes()[ids[1]].metadata().metadata(), &multiplicationNode().metadata());
	BOOST_REQUIRE_EQUAL(g.nodes()[ids[1]].name(), "mult_1");
	BOOST_REQUIRE_EQUAL(&g.nodes()[ids[2]].metadata().metadata(), &additionNode().metadata());
	BOOST_REQUIRE_EQUAL(g.nodes()[ids[2]].name(), "add_2");

	BOOST_CHECK_EQUAL(s_nodeCount, 3u);

	/////
	// remove one node (mult_1)

	BOOST_REQUIRE_NO_THROW(g.nodes().erase(g.nodes().find(ids[1])));
	ids.erase(ids.begin()+1);

	BOOST_REQUIRE(not g.empty());
	BOOST_REQUIRE(not g.nodes().empty());
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 2u);
	BOOST_REQUIRE(g.nodes().begin() != g.nodes().end());

	{
		auto it = g.nodes().begin();
		for(size_t a = 0; a < 2; ++a)
			BOOST_REQUIRE_EQUAL(&(g.nodes()[ids[a]]), &(*(it++)));
	}

	BOOST_REQUIRE_EQUAL(&g.nodes()[ids[0]].metadata().metadata(), &additionNode().metadata());
	BOOST_REQUIRE_EQUAL(g.nodes()[ids[0]].name(), "add_1");
	BOOST_REQUIRE_EQUAL(&g.nodes()[ids[1]].metadata().metadata(), &additionNode().metadata());
	BOOST_REQUIRE_EQUAL(g.nodes()[ids[1]].name(), "add_2");

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
