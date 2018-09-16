#include <boost/test/unit_test.hpp>

#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/graph.h>
#include <dependency_graph/port.inl>
#include <dependency_graph/values.inl>
#include <dependency_graph/metadata_register.h>

#include <actions/app.h>
#include <actions/actions.h>

#include "common.h"

using namespace dependency_graph;

BOOST_AUTO_TEST_CASE(simple_subnet_values) {
	// make the app "singleton"
	possumwood::AppCore app;

	// This whole test is split into two arrays of functors:
	//   - the "command" array, performing the actual action
	//   - the "test" array, doing checks on the output
	// The actual test just iterates over these and tests that the commands work,
	// the result passes the tests, and that undo/redo always leads to consistent results.
	std::vector<std::function<void()>> commands;
	std::vector<std::function<void()>> tests;

	///

	// initial state
	commands.push_back([]() {
	});

	tests.push_back([&]() {
		BOOST_CHECK(app.instance().graph().empty());
	});

	///

	const UniqueId netId;
	dependency_graph::Network* network = nullptr;
	auto netIt = app.graph().nodes().end();

	// create a subnetwork in the graph, via actions
	commands.push_back([&]() {
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(
			app.graph(),
			dependency_graph::MetadataRegister::singleton()["network"],
			"network",
			possumwood::NodeData(),
			netId
		));
	});

	// empty network state test
	tests.push_back([&]() {
		netIt = app.graph().nodes().find(netId);
		BOOST_REQUIRE(netIt != app.graph().nodes().end());
		BOOST_REQUIRE(netIt->is<dependency_graph::Network>());

		network = &netIt->as<dependency_graph::Network>();

		BOOST_CHECK(network->nodes().empty());
		BOOST_CHECK(network->connections().empty());
		BOOST_CHECK_EQUAL(network->metadata()->attributeCount(), 0u);
	});

	///

	const UniqueId inId;
	auto inIt = app.graph().nodes().end();
	dependency_graph::NodeBase* input = nullptr;

	// create a tiny subnetwork
	commands.push_back([&]() {
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(
			*network,
			dependency_graph::MetadataRegister::singleton()["input"],
			"network_input",
			possumwood::NodeData(),
			inId
		));
	});

	tests.push_back([&]() {
		inIt = network->nodes().find(inId);
		BOOST_REQUIRE(inIt != network->nodes().end());
		input = &(*inIt);
	});

	///

	const UniqueId outId;
	auto outIt = app.graph().nodes().end();
	dependency_graph::NodeBase* output = nullptr;

	commands.push_back([&]() {
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(
			*network,
			dependency_graph::MetadataRegister::singleton()["output"],
			"network_output",
			possumwood::NodeData(),
			outId
		));
	});

	tests.push_back([&]() {
		outIt = network->nodes().find(outId);
		BOOST_REQUIRE(outIt != network->nodes().end());
		output = &(*outIt);
	});

	///

	const UniqueId midId;
	auto midIt = app.graph().nodes().end();
	dependency_graph::NodeBase* middle = nullptr;

	commands.push_back([&]() {
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(
			*network,
			passThroughNode(),
			"passthru",
			possumwood::NodeData(),
			midId
		));
	});

	tests.push_back([&]() {
		midIt = network->nodes().find(midId);
		BOOST_REQUIRE(midIt != network->nodes().end());
		middle = &(*midIt);

		// before connecting the input and output, the network should have no ports
		BOOST_CHECK_EQUAL(network->portCount(), 0u);

		/////

		// check that the passthru node works as expected (not using the undo stack)
		BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 0.0f);
		BOOST_CHECK_EQUAL(middle->port(1).get<float>(), 0.0f);
	});

	commands.push_back([&]() {
		BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(middle->port(0), 3.0f));
	});

	tests.push_back([&]() {
		BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 3.0f);
		BOOST_CHECK_EQUAL(middle->port(1).get<float>(), 3.0f);
	});

	commands.push_back([&]() {
		// for the next test, lets set the input to a different value, but don't pull on out yet
		BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(middle->port(0), 5.0f));
	});

	tests.push_back([&]() {
		BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 5.0f);
		BOOST_CHECK_EQUAL(middle->port(1).get<float>(), 5.0f);

		BOOST_CHECK(not middle->port(0).isDirty());
		BOOST_CHECK(not middle->port(1).isDirty());

		// we will be connecting next, for now there are no connections
		BOOST_CHECK_EQUAL(network->connections().size(), 0u);

		// and getting values from input and output should throw an exception
		BOOST_CHECK_THROW(input->port(0).get<float>(), std::exception);
		BOOST_CHECK_THROW(output->port(0).get<float>(), std::exception);
	});

	/////

	commands.push_back([&]() {
		// connect the input
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(input->port(0), middle->port(0)));
	});

	tests.push_back([&]() {
		BOOST_CHECK_EQUAL(app.instance().graph().connections().size(), 0u);
		BOOST_CHECK_EQUAL(network->connections().size(), 1u);

		BOOST_CHECK_EQUAL(input->port(0).get<float>(), 5.0f); // transferred from the connected port
		BOOST_CHECK_THROW(output->port(0).get<float>(), std::exception);

		BOOST_CHECK(not output->port(0).isDirty());
		BOOST_CHECK(middle->port(0).isDirty());
		BOOST_CHECK(middle->port(1).isDirty());
	});


	commands.push_back([&]() {
		// connect the output
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(middle->port(1), output->port(0)));
	});

	tests.push_back([&]() {
		// connecting should create two ports on the network
		BOOST_CHECK_EQUAL(network->metadata()->attributeCount(), 2u);
		BOOST_CHECK_EQUAL(network->portCount(), 2u);

		BOOST_CHECK_EQUAL(network->port(0).type(), dependency_graph::unmangledTypeId<float>());
		BOOST_CHECK_EQUAL(network->port(0).category(), dependency_graph::Attr::kInput);
		BOOST_CHECK_EQUAL(network->port(1).type(), dependency_graph::unmangledTypeId<float>());
		BOOST_CHECK_EQUAL(network->port(1).category(), dependency_graph::Attr::kOutput);

	/////

		// check that the connected passthru works as expected (internal connections only)
		//  -> value was set before connecting, and should be transferred to input/output
		BOOST_CHECK_EQUAL(input->port(0).get<float>(), 5.0f);
		// BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 5.0f);
		// BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 5.0f);
		BOOST_CHECK_EQUAL(output->port(0).get<float>(), 5.0f);

		// external connections should hold the same value now, assuming everything works
		BOOST_CHECK_EQUAL(network->port(0).get<float>(), 5.0f);
		BOOST_CHECK_EQUAL(network->port(1).get<float>(), 5.0f);
	});


	// set a value on the external connection
	commands.push_back([&]() {
		BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(network->port(0), 3.0f));
	});

	tests.push_back([&]() {
		BOOST_CHECK_EQUAL(network->port(0).get<float>(), 3.0f);

		// which should propagate to the internal connections straight away
		BOOST_CHECK_EQUAL(input->port(0).get<float>(), 3.0f);
		BOOST_CHECK_EQUAL(output->port(0).get<float>(), 3.0f);

		// and should also change the output
		BOOST_CHECK_EQUAL(network->port(1).get<float>(), 3.0f);
	});

	/////

	UniqueId out2Id;
	auto out2It = network->nodes().end();
	dependency_graph::NodeBase* output2 = nullptr;

	commands.push_back([&]() {
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(
			*network,
			dependency_graph::MetadataRegister::singleton()["output"],
			"network_output_2",
			possumwood::NodeData(),
			out2Id
		));
	});

	tests.push_back([&]() {
		auto out2It = network->nodes().find(out2Id);
		BOOST_REQUIRE(out2It != network->nodes().end());
		output2 = &(*out2It);

		// making a new output node without connecting it does nothing
		BOOST_CHECK_EQUAL(network->metadata()->attributeCount(), 2u);
		BOOST_CHECK_EQUAL(network->portCount(), 2u);
	});

	commands.push_back([&]() {
		// now connect it
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(middle->port(1), output2->port(0)));
	});

	tests.push_back([&]() {
		// making a new output node creates a second output
		BOOST_CHECK_EQUAL(network->metadata()->attributeCount(), 3u);
		BOOST_CHECK_EQUAL(network->portCount(), 3u);

		// which should have the right value
		BOOST_CHECK_EQUAL(network->port(2).get<float>(), 3.0f);
		BOOST_CHECK_EQUAL(output2->port(0).get<float>(), 3.0f);
	});

	/////

	commands.push_back([&]() {
		// now, disconnecting the first output should just remove an output port
		BOOST_REQUIRE_NO_THROW(possumwood::actions::disconnect(middle->port(1), output->port(0)));
	});

	tests.push_back([&]() {
		// one less attribute
		BOOST_CHECK_EQUAL(network->metadata()->attributeCount(), 2u);
		BOOST_CHECK_EQUAL(network->portCount(), 2u);

		// but the remaining attribute still has the right value
		BOOST_CHECK_EQUAL(network->port(1).get<float>(), 3.0f);
	});

	commands.push_back([&]() {
		// and setting that value works as expected
		BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(network->port(0), 9.0f));
	});

	tests.push_back([&]() {
		BOOST_CHECK_EQUAL(network->port(0).get<float>(), 9.0f);

		BOOST_CHECK_EQUAL(input->port(0).get<float>(), 9.0f);
		BOOST_CHECK_EQUAL(output2->port(0).get<float>(), 9.0f);

		BOOST_CHECK_EQUAL(network->port(1).get<float>(), 9.0f);

		// getting a value from an unconnected port should throw
		BOOST_CHECK_THROW(output->port(0).get<float>(), std::runtime_error);
	});

	/////

	commands.push_back([&]() {
		// disconnecting the input will lead to a network with single output
		BOOST_REQUIRE_NO_THROW(possumwood::actions::disconnect(input->port(0), middle->port(0)));
	});

	tests.push_back([&]() {
		// one less attribute
		BOOST_CHECK_EQUAL(network->metadata()->attributeCount(), 1u);
		BOOST_CHECK_EQUAL(network->portCount(), 1u);

		// but the remaining attribute still has the right value
		BOOST_CHECK_EQUAL(network->port(0).get<float>(), 9.0f);
	});

	// and setting that value works as expected
	commands.push_back([&]() {
		BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(middle->port(0), 13.0f));
	});

	tests.push_back([&]() {
		BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 13.0f);
		BOOST_CHECK_EQUAL(output2->port(0).get<float>(), 13.0f);

		BOOST_CHECK_EQUAL(network->port(0).get<float>(), 13.0f);

		// getting a value from an unconnected port should throw
		BOOST_CHECK_THROW(input->port(0).get<float>(), std::runtime_error);
	});

	/////

	commands.push_back([&]() {
		// connect back the input
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(input->port(0), middle->port(0)));
	});

	tests.push_back([&]() {
		// one less attribute
		BOOST_CHECK_EQUAL(network->metadata()->attributeCount(), 2u);
		BOOST_CHECK_EQUAL(network->portCount(), 2u);

		// but the remaining attribute still has the right value
		BOOST_CHECK_EQUAL(network->port(0).get<float>(), 13.0f);
		BOOST_CHECK_EQUAL(network->port(1).get<float>(), 13.0f);
	});

	commands.push_back([&]() {
		// setting the input value still works
		BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(network->port(0), 15.0f));
	});

	tests.push_back([&]() {
		BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 15.0f);
		BOOST_CHECK_EQUAL(output2->port(0).get<float>(), 15.0f);

		BOOST_CHECK_EQUAL(network->port(0).get<float>(), 15.0f);

		// getting a value from an unconnected port should throw
		BOOST_CHECK_EQUAL(input->port(0).get<float>(), 15.0f);
	});

	const UniqueId indirectInputId;
	dependency_graph::NodeBase* indirectInput = nullptr;
	auto indirectInputIt = app.graph().nodes().end();

	commands.push_back([&]() {
		// make a passthru node
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(
			app.graph(),
			passThroughNode(),
			"passthru",
			possumwood::NodeData(),
			indirectInputId
		));
	});

	tests.push_back([&]() {
		indirectInputIt = app.graph().nodes().find(indirectInputId);
		BOOST_REQUIRE(indirectInputIt != app.graph().nodes().end());
		indirectInput = &(*indirectInputIt);

		BOOST_CHECK_EQUAL(indirectInput->port(0).get<float>(), 0.0f);
		BOOST_CHECK_EQUAL(indirectInput->port(1).get<float>(), 0.0f);
	});

	commands.push_back([&]() {
		// connect back the input
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(indirectInput->port(1), network->port(0)));
	});

	tests.push_back([&]() {
		// the connection to the input node should now exist
		BOOST_CHECK(indirectInput->port(1).isConnected());
		BOOST_REQUIRE(app.graph().connections().connectedFrom(network->port(0)));
		BOOST_CHECK_EQUAL(&(*app.graph().connections().connectedFrom(network->port(0))), &(indirectInput->port(1)));

		// the indirect input node should still be the same
		BOOST_CHECK_EQUAL(indirectInput->port(0).get<float>(), 0.0f);
		BOOST_CHECK_EQUAL(indirectInput->port(1).get<float>(), 0.0f);

		// which means its value should propagate
		BOOST_CHECK_EQUAL(network->port(0).get<float>(), 0.0f);
		BOOST_CHECK_EQUAL(input->port(0).get<float>(), 0.0f);
		BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 0.0f);
		BOOST_CHECK_EQUAL(middle->port(1).get<float>(), 0.0f);
		BOOST_CHECK_EQUAL(output2->port(0).get<float>(), 0.0f);
		BOOST_CHECK_EQUAL(network->port(1).get<float>(), 0.0f);
	});

	commands.push_back([&]() {
		// setting a value on the connected input
		BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(indirectInput->port(0), 23.0f));
	});

	tests.push_back([&]() {
		// the new value should propagate throughout the network
		BOOST_CHECK_EQUAL(network->port(1).get<float>(), 23.0f);

		BOOST_CHECK_EQUAL(indirectInput->port(0).get<float>(), 23.0f);
		BOOST_CHECK_EQUAL(indirectInput->port(1).get<float>(), 23.0f);

		BOOST_CHECK_EQUAL(network->port(0).get<float>(), 23.0f);
		BOOST_CHECK_EQUAL(input->port(0).get<float>(), 23.0f);
		BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 23.0f);
		BOOST_CHECK_EQUAL(middle->port(1).get<float>(), 23.0f);
		BOOST_CHECK_EQUAL(output2->port(0).get<float>(), 23.0f);
		BOOST_CHECK_EQUAL(network->port(1).get<float>(), 23.0f);
	});

	/////

	// execute all actions + tests afterwards
	assert(commands.size() == tests.size());

	for(std::size_t i=0; i<commands.size(); ++i) {
		commands[i]();
		tests[i]();

		BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

		if(i > 0)
			tests[i-1]();

		BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

		tests[i]();
	}

	// simple and silly way of testing Undo + Redo:
	//   - check stack
	//   - do undo
	//   - check state + stack
	//   - do redo
	//   - check state + stack
	//   - do undo, finally (leaves the last action undone)
	auto checkUndo = [&](std::function<void()> testBefore, std::function<void()> testAfter, std::size_t undoActionCount, std::size_t redoActionCount) {
		testBefore();

		BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), undoActionCount);
		BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), redoActionCount);

		BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

		testAfter();

		BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), undoActionCount-1);
		BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), redoActionCount+1);

		BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

		testBefore();

		BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), undoActionCount);
		BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), redoActionCount);

		BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

		testAfter();
	};

	for(std::size_t i = commands.size()-1; i > 0; --i)
		checkUndo(tests[i], tests[i-1], i, commands.size()-i-1);
}
