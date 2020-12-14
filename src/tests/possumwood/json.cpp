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
using nlohmann::json;

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

BOOST_AUTO_TEST_CASE(simple_graph_saving) {
	auto filesystem = std::make_shared<possumwood::FilesystemMock>();

	possumwood::App app(filesystem);

	// make sure the static handles are initialised
	additionNode();
	multiplicationNode();

	// empty serialization
	{
		const json result = "{\"nodes\":{},\"connections\":[], \"name\":\"network\", \"type\":\"network\"}"_json;

		{
			assert(!app.graph().hasParentNetwork());

			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("empty.psw"), false));

			BOOST_CHECK_EQUAL(result, readJson(*filesystem, "empty.psw"));
		}

		{
			BOOST_REQUIRE_NO_THROW(app.loadFile(possumwood::Filepath::fromString("empty.psw")));

			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("empty_too.psw"), false));
			BOOST_CHECK_EQUAL(result, readJson(*filesystem, "empty_too.psw"));
		}
	}

	// a single node, no connections, no blind data
	{
		NodeBase& a = app.graph().nodes().add(additionNode(), "add");

		const json result({{"nodes",
		                    {{"addition_0",
		                      {{"name", "add"},
		                       {"type", "addition"},
		                       {"ports", {{"input_1", 2.0}, {"input_2", 4.0}}}}}}},
		                   {"connections", "[]"_json},
						   {"name", "network"},
						   {"type", "network"}});

		a.port(0).set<float>(2.0f);
		a.port(1).set<float>(4.0f);

		{
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("single.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "single.psw"), result);

			BOOST_REQUIRE_NO_THROW(app.loadFile(possumwood::Filepath::fromString("single.psw")));
		}

		{
			::json json2;
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("single_too.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "single_too.psw"), result);
		}
	}

	// three nodes, a connection, blind data
	{
		NodeBase& m = app.graph().nodes().add(multiplicationNode(), "mult");
		app.graph().nodes().add(multiplicationNode(), "mult");

		const json result(
		    {{"nodes",
		      {{"addition_0", {{"name", "add"}, {"type", "addition"}, {"ports", {{"input_1", 2.0}, {"input_2", 4.0}}}}},
		       {"multiplication_0",
		        {{"name", "mult"},
		         {"type", "multiplication"},
		         {"ports", {{"input_2", 5.0}}},
		         {"blind_data", {{"type", unmangledTypeId<std::string>()}, {"value", "test blind data"}}}}},
		       {"multiplication_1",
		        {{"name", "mult"}, {"type", "multiplication"}, {"ports", {{"input_1", 0.0}, {"input_2", 0.0}}}}}}},
		     {"connections",
		      {{{"in_node", "multiplication_0"},
		        {"in_port", "input_1"},
		        {"out_node", "addition_0"},
		        {"out_port", "output"}}}},
		     {"name", "network"},
		     {"type", "network"}});

		m.port(0).set<float>(3.0f);
		m.port(1).set<float>(5.0f);

		findNode("add").port(2).connect(m.port(0));

		m.setBlindData<std::string>("test blind data");

		{
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("three_nodes.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "three_nodes.psw"), result);

			BOOST_REQUIRE_NO_THROW(app.loadFile(possumwood::Filepath::fromString("three_nodes.psw")));
		}

		{
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("three_nodes_too.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "three_nodes_too.psw"), result);
		}
	}
}

BOOST_AUTO_TEST_CASE(nested_graph_saving) {
	auto filesystem = std::make_shared<possumwood::FilesystemMock>();

	possumwood::App app(filesystem);

	// empty network serialization
	{
		// add an empty network
		{
			auto networkFactoryIterator = MetadataRegister::singleton().find("network");
			BOOST_REQUIRE(networkFactoryIterator != MetadataRegister::singleton().end());

			NodeBase& base = app.graph().nodes().add(*networkFactoryIterator, "test_network");

			BOOST_REQUIRE(base.is<Network>());
		}

		const json result({{
		                       "nodes",
		                       {{"network_0",
		                         {{"name", "test_network"},
		                          {"type", "network"},
		                          {"nodes", "{}"_json},
		                          {"connections", "[]"_json}}}},
		                   },
		                   {"connections", "[]"_json},
		                   {"name", "network"},
		                   {"type", "network"}});

		{
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("network.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "network.psw"), result);

			BOOST_REQUIRE_NO_THROW(app.loadFile(possumwood::Filepath::fromString("network.psw")));
		}

		{
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("network_too.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "network_too.psw"), result);
		}
	}

	// a single node, no connections, no blind data
	{
		NodeBase& a = findNode("test_network").as<dependency_graph::Network>().nodes().add(additionNode(), "add");

		const json result(
		    {{
		         "nodes",
		         {{"network_0",
		           {{"name", "test_network"},
		            {"type", "network"},
		            // {"ports", {}},
		            {"nodes",
		             {{"addition_0",
		               {{"name", "add"}, {"type", "addition"}, {"ports", {{"input_1", 2.0}, {"input_2", 4.0}}}}}}},
		            {"connections", "[]"_json}}}},
		     },
		     {"connections", "[]"_json},
		     {"name", "network"},
		     {"type", "network"}});

		a.port(0).set<float>(2.0f);
		a.port(1).set<float>(4.0f);

		{
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("single_node.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "single_node.psw"), result);

			BOOST_REQUIRE_NO_THROW(app.loadFile(possumwood::Filepath::fromString("single_node.psw")));
		}

		{
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("single_node_too.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "single_node_too.psw"), result);
		}
	}

	// three nodes, a connection, blind data
	{
		auto& net = findNode("test_network").as<dependency_graph::Network>();

		NodeBase& m = net.nodes().add(multiplicationNode(), "mult");
		net.nodes().add(multiplicationNode(), "mult");

		const json result(
		    {{
		         "nodes",
		         {{"network_0",
		           {{"name", "test_network"},
		            {"type", "network"},
		            // {"ports", {}},
		            {"nodes",
		             {{"addition_0",
		               {{"name", "add"}, {"type", "addition"}, {"ports", {{"input_1", 2.0}, {"input_2", 4.0}}}}},
		              {"multiplication_0",
		               {{"name", "mult"},
		                {"type", "multiplication"},
		                {"ports", {{"input_2", 5.0}}},
		                {"blind_data", {{"type", unmangledTypeId<std::string>()}, {"value", "test blind data"}}}}},
		              {"multiplication_1",
		               {{"name", "mult"},
		                {"type", "multiplication"},
		                {"ports", {{"input_1", 0.0}, {"input_2", 0.0}}}}}}},
		            {"connections",
		             {{{"in_node", "multiplication_0"},
		               {"in_port", "input_1"},
		               {"out_node", "addition_0"},
		               {"out_port", "output"}}}}}}},
		     },
		     {"connections", "[]"_json},
		     {"name", "network"},
		     {"type", "network"}});

		m.port(0).set<float>(3.0f);
		m.port(1).set<float>(5.0f);

		findNode(net, "add").port(2).connect(m.port(0));

		m.setBlindData<std::string>("test blind data");

		{
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("blind_data.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "blind_data.psw"), result);

			BOOST_REQUIRE_NO_THROW(app.loadFile(possumwood::Filepath::fromString("blind_data.psw")));
		}

		{
			BOOST_REQUIRE_NO_THROW(app.saveFile(possumwood::Filepath::fromString("blind_data_too.psw"), false));
			BOOST_CHECK_EQUAL(readJson(*filesystem, "blind_data_too.psw"), result);
		}
	}
}
