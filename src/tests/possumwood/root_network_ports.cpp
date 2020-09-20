#include <actions/actions.h>
#include <actions/filesystem_mock.h>
#include <dependency_graph/graph.h>
#include <dependency_graph/metadata_register.h>
#include <dependency_graph/rtti.h>
#include <possumwood_sdk/app.h>

#include <boost/test/unit_test.hpp>

#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes.inl>
#include <dependency_graph/port.inl>

#include "common.h"

using namespace dependency_graph;
using possumwood::io::json;

namespace {

dependency_graph::NodeBase& findNode(dependency_graph::Network& net, const std::string& name) {
	for(auto& n : net.nodes())
		if(n.name() == name)
			return n;

	BOOST_REQUIRE(false && "Node not found, fail");
	throw;
}

dependency_graph::NodeBase& findNode(const std::string& name) {
	return findNode(possumwood::AppCore::instance().graph(), name);
}

}  // namespace

BOOST_AUTO_TEST_CASE(root_network_ports) {
	possumwood::App app;

	auto inputMeta = dependency_graph::MetadataRegister::singleton().find("input");
	BOOST_REQUIRE(inputMeta != dependency_graph::MetadataRegister::singleton().end());

	auto outputMeta = dependency_graph::MetadataRegister::singleton().find("output");
	BOOST_REQUIRE(outputMeta != dependency_graph::MetadataRegister::singleton().end());

	BOOST_REQUIRE_NO_THROW(
	    possumwood::actions::createNode(app.graph(), passThroughNode(), "passthru", possumwood::NodeData()));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(app.graph(), *inputMeta, "input", possumwood::NodeData()));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(app.graph(), *outputMeta, "output", possumwood::NodeData()));

	auto& in = findNode("input");
	auto& out = findNode("output");
	auto& pass = findNode("passthru");

	// before connections - the network should have no inputs or outputs
	BOOST_CHECK_EQUAL(app.graph().portCount(), 0u);

	BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(in.port(0), pass.port(0)));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(pass.port(1), out.port(0)));

	// after connections - the network should have one input and one output
	BOOST_CHECK_EQUAL(app.graph().portCount(), 2u);

	// check the port types
	BOOST_CHECK_EQUAL(app.graph().port(0).type().name(), typeid(float).name());
	BOOST_CHECK_EQUAL(app.graph().port(1).type().name(), typeid(float).name());

	BOOST_CHECK_EQUAL(app.graph().port(0).category(), Attr::Category::kInput);
	BOOST_CHECK_EQUAL(app.graph().port(1).category(), Attr::Category::kOutput);

	// let's test the functionality
	BOOST_CHECK_EQUAL(app.graph().port(0).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(app.graph().port(1).get<float>(), 0.0f);

	BOOST_REQUIRE_NO_THROW(app.graph().port(0).set<float>(2.0f));

	BOOST_CHECK_EQUAL(app.graph().port(0).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(app.graph().port(1).get<float>(), 2.0f);

	// undo 7 actions back to the beginning
	for(int a = 0; a < 7; ++a)
		BOOST_CHECK_NO_THROW(app.undoStack().undo());

	// the scene should be empty now
	BOOST_CHECK_EQUAL(app.graph().nodes().size(), 0u);
	BOOST_CHECK_EQUAL(app.graph().portCount(), 0u);

	// redo 7 actions back to the end
	for(int a = 0; a < 7; ++a)
		BOOST_CHECK_NO_THROW(app.undoStack().redo());

	// after connections - the network should have one input and one output
	BOOST_CHECK_EQUAL(app.graph().portCount(), 2u);

	// check the port types
	BOOST_CHECK_EQUAL(app.graph().port(0).type().name(), typeid(float).name());
	BOOST_CHECK_EQUAL(app.graph().port(1).type().name(), typeid(float).name());

	BOOST_CHECK_EQUAL(app.graph().port(0).category(), Attr::Category::kInput);
	BOOST_CHECK_EQUAL(app.graph().port(1).category(), Attr::Category::kOutput);

	// let's test the functionality
	BOOST_CHECK_EQUAL(app.graph().port(0).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(app.graph().port(1).get<float>(), 0.0f);

	BOOST_REQUIRE_NO_THROW(app.graph().port(0).set<float>(2.0f));

	BOOST_CHECK_EQUAL(app.graph().port(0).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(app.graph().port(1).get<float>(), 2.0f);
}
