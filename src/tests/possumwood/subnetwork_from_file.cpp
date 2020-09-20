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

json readJson(possumwood::IFilesystem& filesystem, const std::string& filename) {
	json result;

	auto stream = filesystem.read(possumwood::Filepath::fromString(filename));

	(*stream) >> result;

	return result;
}

}  // namespace

BOOST_AUTO_TEST_CASE(network_from_file) {
	auto filesystem = std::make_shared<possumwood::FilesystemMock>();

	possumwood::App app(filesystem);
	dependency_graph::State state;

	// make sure the static handles are initialised
	passThroughNode();

	// three nodes, a connection, blind data
	const json subnetwork({
	    {"nodes",
	     {{"input_0", {{"name", "this_is_an_input"}, {"type", "input"}}},
	      {"pass_through_0", {{"name", "pass_through"}, {"type", "pass_through"}}},
	      {"output_0", {{"name", "this_is_an_output"}, {"type", "output"}}}}},
	    {"connections",
	     {{{"in_node", "pass_through_0"}, {"in_port", "input"}, {"out_node", "input_0"}, {"out_port", "data"}},
	      {{"in_node", "output_0"}, {"in_port", "data"}, {"out_node", "pass_through_0"}, {"out_port", "output"}}}},
	});

	(*filesystem->write(possumwood::Filepath::fromString("subnetwork_only.psw"))) << subnetwork;

	BOOST_REQUIRE_NO_THROW(state = app.loadFile(possumwood::Filepath::fromString("subnetwork_only.psw")));
	BOOST_REQUIRE(!state.errored());

	// lets make sure this loads and saves correctly
	BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("subnetwork_only_too.psw"), false));
	BOOST_CHECK_EQUAL(readJson(*filesystem, "subnetwork_only_too.psw"), subnetwork);

	BOOST_REQUIRE_NO_THROW(app.loadFile(possumwood::Filepath::fromString("subnetwork_only_too.psw")));

	/////////////
	// attempt to serialize a simple setup with a subnetwork
	{
		json setup;
		BOOST_REQUIRE_NO_THROW(setup = json({{"nodes",
		                                      {{"network_0",
		                                        {{"name", "network"},
		                                         {"type", "network"},
		                                         {"nodes", subnetwork["nodes"]},
		                                         {"connections", subnetwork["connections"]},
		                                         {"ports", {{"this_is_an_input", 0.0}}}}}}},
		                                     {"connections", std::vector<std::string>()}}));

		(*filesystem->write(possumwood::Filepath::fromString("setup_with_subnetwork.psw"))) << setup;

		BOOST_REQUIRE_NO_THROW(state = app.loadFile(possumwood::Filepath::fromString("setup_with_subnetwork.psw")));
		std::cout << state << std::endl;
		BOOST_REQUIRE(!state.errored());

		// lets make sure this loads and saves correctly
		BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("setup_with_subnetwork_too.psw"), false));
		BOOST_CHECK_EQUAL(readJson(*filesystem, "setup_with_subnetwork_too.psw"), setup);

		BOOST_REQUIRE_NO_THROW(app.loadFile(possumwood::Filepath::fromString("setup_with_subnetwork_too.psw")));
	}

	//////////////
	// make a copy of that setup and try to set the source attribute - represents a network coming
	// from another file
	{
		json setup;
		BOOST_REQUIRE_NO_THROW(setup = json({{"nodes",
		                                      {{"network_0",
		                                        {{"name", "network"},
		                                         {"type", "network"},
		                                         {"source", "subnetwork_only.psw"},
		                                         {"ports", {{"this_is_an_input", 0.0}}}}}}},
		                                     {"connections", std::vector<std::string>()}}));

		(*filesystem->write(possumwood::Filepath::fromString("setup_without_subnetwork.psw"))) << setup;

		BOOST_REQUIRE_NO_THROW(state = app.loadFile(possumwood::Filepath::fromString("setup_without_subnetwork.psw")));
		BOOST_REQUIRE(!state.errored());

		// lets make sure this loads and saves correctly
		BOOST_REQUIRE_NO_THROW(
		    app.saveFile(possumwood::Filepath::fromString("setup_without_subnetwork_too.psw"), false));
		BOOST_CHECK_EQUAL(readJson(*filesystem, "setup_without_subnetwork_too.psw"), setup);

		BOOST_REQUIRE_NO_THROW(app.loadFile(possumwood::Filepath::fromString("setup_without_subnetwork_too.psw")));
	}

	// check that we can set values on the network
	auto& net = findNode("network").as<dependency_graph::Network>();
	BOOST_CHECK_EQUAL(net.portCount(), 2u);
	BOOST_CHECK_EQUAL(net.port(0).name(), "this_is_an_input");
	BOOST_CHECK_EQUAL(net.port(0).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(net.port(1).name(), "this_is_an_output");
	BOOST_CHECK_EQUAL(net.port(1).get<float>(), 0.0f);

	// test that the eval still works
	BOOST_CHECK_NO_THROW(net.port(0).set(5.0f));
	BOOST_CHECK_EQUAL(net.port(0).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(net.port(1).get<float>(), 5.0f);
}
