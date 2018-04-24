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

namespace dependency_graph {

static std::ostream& operator << (std::ostream& out, const UniqueId& id) {
	out << "(id)";
	return out;
}

static std::ostream& operator << (std::ostream& out, const Nodes::iterator& i) {
	out << "(it)";
	return out;
}

}

using namespace dependency_graph;

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

	// make a total list of node IDs
	std::set<UniqueId> nodeIDs;
	nodeIDs.insert(network.index());
	nodeIDs.insert(n1.index());
	nodeIDs.insert(n2.index());
	nodeIDs.insert(n3.index());
	nodeIDs.insert(n4.index());

	BOOST_REQUIRE_EQUAL(nodeIDs.size(), 5);

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
