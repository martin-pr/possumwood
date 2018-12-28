#include <boost/test/unit_test.hpp>

#include <iostream>

#include <dependency_graph/graph.h>
#include <dependency_graph/node.h>
#include <dependency_graph/nodes.inl>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/metadata_register.h>

#include "common.h"

using namespace dependency_graph;

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

bool checkConnections(const Network& n, std::vector<std::pair<const Port*, const Port*>>& connections) {
	bool result = true;

	for(auto c : n.connections()) {
		std::pair<const Port*, const Port*> key(&c.first, &c.second);

		auto it = std::find(connections.begin(), connections.end(), key);
		if(it == connections.end())
			result = false;
		else
			connections.erase(it);
	}

	if(result) {
		for(auto& node : n.nodes())
			if(node.is<Network>())
				result = result & checkConnections(node.as<Network>(), connections);
	}

	return result;
}

bool checkConnections(const Graph& g, const std::vector<std::pair<const Port*, const Port*>>& connections) {
	std::vector<std::pair<const Port*, const Port*>> tmp = connections;

	const bool result = checkConnections((const Network&)g, tmp);

	return result && tmp.empty();
}

}

BOOST_AUTO_TEST_CASE(network_connections) {
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

	// try to connect outside nested network
	BOOST_CHECK_NO_THROW(na1.port(2).connect(na2.port(0)));

	BOOST_CHECK(checkConnections(g, {{
		std::make_pair(&na1.port(2), &na2.port(0))
	}}));

	// try to connect inside nested network
	BOOST_CHECK_NO_THROW(nb1.port(2).connect(nb2.port(0)));

	BOOST_CHECK(checkConnections(g, {{
		std::make_pair(&na1.port(2), &na2.port(0)),
		std::make_pair(&nb1.port(2), &nb2.port(0))
	}}));

	// try to connect between inside and outside network
	BOOST_CHECK_THROW(na1.port(2).connect(nb2.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(nb1.port(2).connect(na2.port(1)), std::runtime_error);

	BOOST_CHECK(checkConnections(g, {{
		std::make_pair(&na1.port(2), &na2.port(0)),
		std::make_pair(&nb1.port(2), &nb2.port(0))
	}}));
}

BOOST_AUTO_TEST_CASE(network_connections_2) {
	Graph g;

	// instantiate nodes from arithmetic.cpp
	const MetadataHandle& addition = additionNode();
	const MetadataHandle& multiplication = multiplicationNode();

	NodeBase& parentNetworkBase = g.nodes().add(MetadataRegister::singleton()["network"], "parent_network");
	BOOST_REQUIRE(parentNetworkBase.is<Network>());
	Network& parentNetwork = parentNetworkBase.as<Network>();

	NodeBase& networkBase = parentNetwork.nodes().add(MetadataRegister::singleton()["network"], "new_network");
	BOOST_REQUIRE(networkBase.is<Network>());
	Network& network = networkBase.as<Network>();

	NodeBase& na1 = g.nodes().add(addition, "add_1");
	NodeBase& na2 = g.nodes().add(multiplication, "mult_1");
	NodeBase& nb_parent = parentNetwork.nodes().add(multiplication, "mult_parent");
	NodeBase& nb1 = network.nodes().add(multiplication, "mult_2");
	NodeBase& nb2 = network.nodes().add(addition, "add_2");

	// 4 nodes inside a network and no connections
	BOOST_REQUIRE_EQUAL(g.nodes().size(), 3u);
	BOOST_REQUIRE_EQUAL(parentNetwork.nodes().size(), 2u);
	BOOST_REQUIRE_EQUAL(network.nodes().size(), 2u);
	BOOST_REQUIRE_EQUAL(g.connections().size(), 0u);
	BOOST_REQUIRE_EQUAL(parentNetwork.connections().size(), 0u);
	BOOST_REQUIRE_EQUAL(network.connections().size(), 0u);

	// check the nodes
	BOOST_CHECK(checkNodes(g, {{&parentNetwork, &na1, &na2}}));
	BOOST_CHECK(checkNodes(parentNetwork, {{ &network, &nb_parent }}));
	BOOST_CHECK(checkNodes(network, {{&nb1, &nb2}}));
	BOOST_CHECK(checkNodes(g, {{&parentNetwork, &network, &nb1, &nb2, &nb_parent, &na1, &na2}}, dependency_graph::Nodes::kRecursive));

	// try to connect outside nested network
	BOOST_CHECK_NO_THROW(na1.port(2).connect(na2.port(0)));

	BOOST_CHECK(checkConnections(g, {{
		std::make_pair(&na1.port(2), &na2.port(0))
	}}));

	// try to connect inside nested network
	BOOST_CHECK_NO_THROW(nb1.port(2).connect(nb2.port(0)));

	BOOST_CHECK(checkConnections(g, {{
		std::make_pair(&na1.port(2), &na2.port(0)),
		std::make_pair(&nb1.port(2), &nb2.port(0))
	}}));

	// try to connect between inside and outside network
	BOOST_CHECK_THROW(na1.port(2).connect(nb2.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(nb1.port(2).connect(na2.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(nb1.port(2).connect(nb_parent.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(nb_parent.port(2).connect(na2.port(1)), std::runtime_error);
	BOOST_CHECK_THROW(nb_parent.port(2).connect(nb2.port(1)), std::runtime_error);

	BOOST_CHECK(checkConnections(g, {{
		std::make_pair(&na1.port(2), &na2.port(0)),
		std::make_pair(&nb1.port(2), &nb2.port(0))
	}}));
}
