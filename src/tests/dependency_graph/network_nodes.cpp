#include <boost/test/unit_test.hpp>

#include <iostream>

#include <dependency_graph/graph.h>
#include <dependency_graph/node.h>
#include <dependency_graph/nodes.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/metadata_register.h>

#include "common.h"

using namespace dependency_graph;

namespace dependency_graph {

static std::ostream& operator << (std::ostream& out, const Nodes::iterator& i) {
	out << "(it " << i->index() << ")";
	return out;
}

}

namespace {

bool checkNodes(const Network& g, std::vector<NodeBase*> nodes, dependency_graph::Nodes::SearchType st = dependency_graph::Nodes::kThisNetwork) {
	auto it1 = g.nodes().begin(st);
	auto it2 = nodes.begin();

	while(it1 != g.nodes().end() || it2 != nodes.end()) {
		if(&(*it1) != *it2)
			return false;

		++it1;
		++it2;
	}

	return it1 == g.nodes().end() && it2 == nodes.end();
}

}

BOOST_AUTO_TEST_CASE(network_nodes_instantiation) {
	Graph g;

	// instantiate nodes from arithmetic.cpp
	const MetadataHandle& addition = additionNode();
	const MetadataHandle& multiplication = multiplicationNode();

	NodeBase& networkBase = g.nodes().add(MetadataRegister::singleton()["network"], "new_network");
	BOOST_REQUIRE(networkBase.is<Network>());
	Network& network = networkBase.as<Network>();

	NodeBase& na1 = g.nodes().add(addition, "add_1");
	NodeBase& na2 = g.nodes().add(multiplication, "mult_1");
	NodeBase& nb1 = network.nodes().add(multiplication, "mult_2");
	NodeBase& nb2 = network.nodes().add(addition, "add_2");

	// 4 nodes inside a network and no connections
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 3u);
	BOOST_REQUIRE_EQUAL(network.nodes().size(), 2u);
	BOOST_REQUIRE_EQUAL(g.connections().size(), 0u);
	BOOST_REQUIRE_EQUAL(network.connections().size(), 0u);

	// check the nodes
	BOOST_CHECK(checkNodes(g, {{&networkBase, &na1, &na2}}));
	BOOST_CHECK(checkNodes(network, {{&nb1, &nb2}}));
	BOOST_CHECK(checkNodes(g, {{&networkBase, &nb1, &nb2, &na1, &na2}}, dependency_graph::Nodes::kRecursive));
}

BOOST_AUTO_TEST_CASE(network_node_iterator) {
	Graph g;

	// instantiate nodes from arithmetic.cpp
	const MetadataHandle& addition = additionNode();
	const MetadataHandle& multiplication = multiplicationNode();

	NodeBase& networkBase = g.nodes().add(MetadataRegister::singleton()["network"], "new_network");
	BOOST_REQUIRE(networkBase.is<Network>());
	Network& network = networkBase.as<Network>();

	NodeBase& n1 = network.nodes().add(addition, "add_1");
	NodeBase& n2 = network.nodes().add(multiplication, "mult_1");
	NodeBase& n3 = network.nodes().add(multiplication, "mult_2");
	NodeBase& n4 = network.nodes().add(addition, "add_2");

	// 4 nodes inside a network and no connections
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 1u);
	BOOST_REQUIRE_EQUAL(network.nodes().size(), 4u);
	BOOST_REQUIRE_EQUAL(g.connections().size(), 0u);
	BOOST_REQUIRE_EQUAL(network.connections().size(), 0u);

	// try non-recursive iteration
	{
		auto it = g.nodes().begin();
		BOOST_CHECK_EQUAL(it->index(), network.index());
		++it;
		BOOST_CHECK_EQUAL(it, g.nodes().end());
	}

	// and try recursive iteration
	{
		auto it = g.nodes().begin(Nodes::kRecursive);
		BOOST_CHECK_EQUAL((it++)->index(), network.index());
		BOOST_CHECK_EQUAL((it++)->index(), n1.index());
		BOOST_CHECK_EQUAL((it++)->index(), n2.index());
		BOOST_CHECK_EQUAL((it++)->index(), n3.index());
		BOOST_CHECK_EQUAL((it++)->index(), n4.index());
		BOOST_CHECK_EQUAL(it, g.nodes().end());
	}

	// try to find "network"
	BOOST_REQUIRE(g.nodes().find(network.index()) != g.nodes().end());
	BOOST_CHECK_EQUAL(&(*g.nodes().find(network.index())), &network);

	// non-recursive find will not find n1
	BOOST_REQUIRE_EQUAL(g.nodes().find(n1.index()), g.nodes().end());

	// but a recursive one will
	BOOST_REQUIRE(g.nodes().find(n1.index(), Nodes::kRecursive) != g.nodes().end());
	BOOST_CHECK_EQUAL(&(*g.nodes().find(n1.index(), Nodes::kRecursive)), &n1);
}
