#include <boost/test/unit_test.hpp>

#include <set>

#include <dependency_graph/graph.h>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/node.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/values.inl>
#include <actions/io/graph.h>
#include <dependency_graph/port.inl>

#include "common.h"

using namespace dependency_graph;

bool checkDirtyPorts(const NodeBase& n, const std::set<size_t>& dirtyPorts) {
	bool result = true;

	for(size_t pi = 0; pi < n.portCount(); ++pi) {
		BOOST_CHECK_EQUAL(n.port(pi).isDirty(), dirtyPorts.find(pi) != dirtyPorts.end());

		if(n.port(pi).isDirty() != (dirtyPorts.find(pi) != dirtyPorts.end()))
			result = false;
	}

	return result;
}

BOOST_AUTO_TEST_CASE(arithmetic) {

	//////////////////
	// addition node

	// metadata for a simple addition node
	const MetadataHandle& addition = additionNode();
	// and for a simple multiplication node
	const MetadataHandle& multiplication = multiplicationNode();

	/////////////////////////////
	// build a simple graph for (a + b) * c + (a + b) * d

	Graph g;

	NodeBase& add1 = g.nodes().add(addition, "add_1");
	NodeBase& mult1 = g.nodes().add(multiplication, "mult_1");
	NodeBase& mult2 = g.nodes().add(multiplication, "mult_2");
	NodeBase& add2 = g.nodes().add(addition, "add_2");

	// before anything is connected, only output ports are dirty (they need to be pulled on)
	for(auto& n : g.nodes())
		for(size_t pi = 0; pi < n.portCount(); ++pi)
			BOOST_CHECK_EQUAL(n.port(pi).isDirty(), n.port(pi).category() == Attr::kOutput);

	// a valid connection
	BOOST_CHECK_NO_THROW(add1.port(2).connect(mult1.port(1)));

	BOOST_CHECK_NO_THROW(add1.port(2).connect(mult2.port(1)));
	BOOST_CHECK_NO_THROW(mult1.port(2).connect(add2.port(0)));
	BOOST_CHECK_NO_THROW(mult2.port(2).connect(add2.port(1)));

	/////////////////////////////
	// compute (2 + 3) * 4 + (2 + 3) * 5 = 20 + 25 = 45

	// at the beginning before any evaluation, only non-connected inputs are not dirty
	for(auto& n : g.nodes())
		for(size_t pi = 0; pi < n.portCount(); ++pi)
			BOOST_CHECK_EQUAL(n.port(pi).isDirty(), n.port(pi).category() == Attr::kOutput || g.connections().connectedFrom(n.port(pi)));

	// check we can get a value from input ports, if they're not connected
	BOOST_REQUIRE_EQUAL(add1.port(0).get<float>(), 0.0f);
	BOOST_REQUIRE_EQUAL(add1.port(1).get<float>(), 0.0f);
	BOOST_REQUIRE_EQUAL(mult1.port(0).get<float>(), 0.0f);
	BOOST_REQUIRE_EQUAL(mult2.port(0).get<float>(), 0.0f);

	// set input values, which makes them not dirty
	BOOST_CHECK_NO_THROW(add1.port(0).set(2.0f));
	BOOST_CHECK_NO_THROW(add1.port(1).set(3.0f));
	BOOST_CHECK_NO_THROW(mult1.port(0).set(4.0f));
	BOOST_CHECK_NO_THROW(mult2.port(0).set(5.0f));

	BOOST_REQUIRE(checkDirtyPorts(add1, {2})); // only output of the add node is dirty
	BOOST_REQUIRE(checkDirtyPorts(mult1, {1, 2})); // first input and output of mult1 node are dirty
	BOOST_REQUIRE(checkDirtyPorts(mult2, {1, 2})); // first input and output of mult2 node are dirty
	BOOST_REQUIRE(checkDirtyPorts(add2, {0, 1, 2})); // all ports of the add2 node are dirty

	// evaluate the whole graph and test output
	BOOST_CHECK_EQUAL(add2.port(2).get<float>(), 45.0f);

	// at the end after any evaluation, everything is NOT dirty
	for(auto& n : g.nodes())
		for(size_t pi = 0; pi < n.portCount(); ++pi)
			BOOST_CHECK(not n.port(pi).isDirty());

	// and test partial outputs
	BOOST_CHECK_EQUAL(add1.port(2).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(mult1.port(2).get<float>(), 20.0f);
	BOOST_CHECK_EQUAL(mult2.port(2).get<float>(), 25.0f);

	////////////////////////////////
	// dirtiness propagation

	// change 2 to 6
	BOOST_CHECK_NO_THROW(add1.port(0).set(6.0f));

	BOOST_REQUIRE(checkDirtyPorts(add1, {2})); // the output of the add node is dirty
	BOOST_REQUIRE(checkDirtyPorts(mult1, {1, 2})); // first input and output of mult1 node are dirty
	BOOST_REQUIRE(checkDirtyPorts(mult2, {1, 2})); // first input and output of mult2 node are dirty
	BOOST_REQUIRE(checkDirtyPorts(add2, {0, 1, 2})); // all ports of the add2 node are dirty

	// compute (6 + 3) * 4 + (6 + 3) * 5 = 36 + 45 = 81
	BOOST_CHECK_EQUAL(add2.port(2).get<float>(), 81.0f);

	// change 4 to 7
	BOOST_CHECK_NO_THROW(mult1.port(0).set(7.0f));

	BOOST_REQUIRE(checkDirtyPorts(add1, {})); // add1 has not been affected by this change
	BOOST_REQUIRE(checkDirtyPorts(mult1, {2})); // the output of mult1 is now dirty
	BOOST_REQUIRE(checkDirtyPorts(mult2, {})); // mult2 has not been affected
	BOOST_REQUIRE(checkDirtyPorts(add2, {0, 2})); // first input and output of the add2 node are dirty

	// compute (6 + 3) * 7 + (6 + 3) * 5 = 63 + 45 = 108
	BOOST_CHECK_EQUAL(add2.port(2).get<float>(), 108.0f);
}
