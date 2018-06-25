#include <boost/test/unit_test.hpp>

#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/graph.h>
#include <dependency_graph/port.inl>
#include <dependency_graph/values.inl>
#include <dependency_graph/metadata_register.h>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/actions.h>

#include "common.h"

using namespace dependency_graph;

BOOST_AUTO_TEST_CASE(simple_subnet_values) {
	// make the app "singleton"
	possumwood::AppCore app;

	// create a subnetwork in the graph, via actions
	UniqueId netId;
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::createNode(
		app.graph(),
		dependency_graph::MetadataRegister::singleton()["network"],
		"network",
		possumwood::NodeData(),
		netId
	));

	auto netIt = app.graph().nodes().find(netId);
	BOOST_REQUIRE(netIt != app.graph().nodes().end());
	BOOST_REQUIRE(netIt->is<dependency_graph::Network>());

	dependency_graph::Network& network = netIt->as<dependency_graph::Network>();

	// initial state check
	BOOST_CHECK(network.nodes().empty());
	BOOST_CHECK(network.connections().empty());
	BOOST_CHECK_EQUAL(network.metadata()->attributeCount(), 0u);

	// create a tiny subnetwork
	UniqueId inId;
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::createNode(
		network,
		dependency_graph::MetadataRegister::singleton()["input"],
		"network_input",
		possumwood::NodeData(),
		inId
	));

	auto inIt = network.nodes().find(inId);
	BOOST_REQUIRE(inIt != network.nodes().end());
	dependency_graph::NodeBase& input = *inIt;

	UniqueId outId;
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::createNode(
		network,
		dependency_graph::MetadataRegister::singleton()["output"],
		"network_output",
		possumwood::NodeData(),
		outId
	));

	auto outIt = network.nodes().find(outId);
	BOOST_REQUIRE(outIt != network.nodes().end());
	dependency_graph::NodeBase& output = *outIt;

	UniqueId midId;
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::createNode(
		network,
		passThroughNode(),
		"passthru",
		possumwood::NodeData(),
		midId
	));

	auto midIt = network.nodes().find(midId);
	BOOST_REQUIRE(midIt != network.nodes().end());
	dependency_graph::NodeBase& middle = *midIt;

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 4u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// before connecting the input and output, the network should have no ports
	BOOST_CHECK_EQUAL(network.portCount(), 0u);

	/////

	// check that the passthru node works as expected (not using the undo stack)
	BOOST_CHECK_EQUAL(middle.port(0).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(middle.port(1).get<float>(), 0.0f);

	BOOST_REQUIRE_NO_THROW(middle.port(0).set(3.0f));

	BOOST_CHECK_EQUAL(middle.port(0).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(middle.port(1).get<float>(), 3.0f);

	// for the next test, lets set the input to a different value, but don't pull on out yet
	BOOST_REQUIRE_NO_THROW(middle.port(0).set(5.0f));

	/////

	// connect the input
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::connect(input.port(0), middle.port(0)));

	// connect the output
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::connect(middle.port(1), output.port(0)));

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 6u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// connecting should create two ports on the network
	BOOST_CHECK_EQUAL(network.metadata()->attributeCount(), 2u);
	BOOST_CHECK_EQUAL(network.portCount(), 2u);

	BOOST_CHECK_EQUAL(network.port(0).type(), dependency_graph::unmangledTypeId<float>());
	BOOST_CHECK_EQUAL(network.port(0).category(), dependency_graph::Attr::kInput);
	BOOST_CHECK_EQUAL(network.port(1).type(), dependency_graph::unmangledTypeId<float>());
	BOOST_CHECK_EQUAL(network.port(1).category(), dependency_graph::Attr::kOutput);

	/////

	// check that the connected passthru works as expected (internal connections only)
	//  -> value was set before connecting, and should be transferred to input/output
	BOOST_CHECK_EQUAL(input.port(0).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(output.port(0).get<float>(), 5.0f);

	// external connections should hold the same value now, assuming everything works
	BOOST_CHECK_EQUAL(network.port(0).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(network.port(1).get<float>(), 5.0f);

	// set a value on the external connection
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::setValue(network.port(0), 3.0f));
	BOOST_CHECK_EQUAL(network.port(0).get<float>(), 3.0f);

	// which should propagate to the internal connections straight away
	BOOST_CHECK_EQUAL(input.port(0).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(output.port(0).get<float>(), 3.0f);

	// and should also change the output
	BOOST_CHECK_EQUAL(network.port(1).get<float>(), 3.0f);

	/////

	UniqueId out2Id;
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::createNode(
		network,
		dependency_graph::MetadataRegister::singleton()["output"],
		"network_output_2",
		possumwood::NodeData(),
		out2Id
	));

	auto out2It = network.nodes().find(out2Id);
	BOOST_REQUIRE(out2It != network.nodes().end());
	dependency_graph::NodeBase& output2 = *out2It;

	// making a new output node without connecting it does nothing
	BOOST_CHECK_EQUAL(network.metadata()->attributeCount(), 2u);
	BOOST_CHECK_EQUAL(network.portCount(), 2u);

	// now connect it
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::connect(middle.port(1), output2.port(0)));

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 9u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// making a new output node creates a second output
	BOOST_CHECK_EQUAL(network.metadata()->attributeCount(), 3u);
	BOOST_CHECK_EQUAL(network.portCount(), 3u);

	// which should have the right value
	BOOST_CHECK_EQUAL(network.port(2).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(output2.port(0).get<float>(), 3.0f);

	/////

	// now, disconnecting the first output should just remove an output port
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::disconnect(middle.port(1), output.port(0)));

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 10u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// one less attribute
	BOOST_CHECK_EQUAL(network.metadata()->attributeCount(), 2u);
	BOOST_CHECK_EQUAL(network.portCount(), 2u);

	// but the remaining attribute still has the right value
	BOOST_CHECK_EQUAL(network.port(1).get<float>(), 3.0f);

	// and setting that value works as expected
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::setValue(network.port(0), 9.0f));
	BOOST_CHECK_EQUAL(network.port(0).get<float>(), 9.0f);

	BOOST_CHECK_EQUAL(input.port(0).get<float>(), 9.0f);
	BOOST_CHECK_EQUAL(output2.port(0).get<float>(), 9.0f);

	BOOST_CHECK_EQUAL(network.port(1).get<float>(), 9.0f);

	// getting a value from an unconnected port should throw
	BOOST_CHECK_THROW(output.port(0).get<float>(), std::runtime_error);

	/////

	// disconnecting the input will lead to a network with single output
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::disconnect(input.port(0), middle.port(0)));

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 12u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// one less attribute
	BOOST_CHECK_EQUAL(network.metadata()->attributeCount(), 1u);
	BOOST_CHECK_EQUAL(network.portCount(), 1u);

	// but the remaining attribute still has the right value
	BOOST_CHECK_EQUAL(network.port(0).get<float>(), 9.0f);

	// and setting that value works as expected
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::setValue(middle.port(0), 13.0f));
	BOOST_CHECK_EQUAL(middle.port(0).get<float>(), 13.0f);
	BOOST_CHECK_EQUAL(output2.port(0).get<float>(), 13.0f);

	BOOST_CHECK_EQUAL(network.port(0).get<float>(), 13.0f);

	// getting a value from an unconnected port should throw
	BOOST_CHECK_THROW(input.port(0).get<float>(), std::runtime_error);
}
