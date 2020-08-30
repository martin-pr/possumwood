#include <actions/actions.h>
#include <dependency_graph/graph.h>
#include <dependency_graph/rtti.h>
#include <possumwood_sdk/app.h>

#include <boost/test/unit_test.hpp>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes_iterator.inl>
#include <dependency_graph/port.inl>
#include <vector>

#include "common.h"

using namespace dependency_graph;
using possumwood::io::json;

namespace {

dependency_graph::Nodes::const_iterator findNode(const dependency_graph::Network& net, const std::string& name) {
	dependency_graph::Nodes::const_iterator result = net.nodes().end();

	for(auto it = net.nodes().begin(); it != net.nodes().end(); ++it)
		if(it->name() == name) {
			result = it;
			break;
		}

	return result;
}

}  // namespace

BOOST_AUTO_TEST_CASE(clipboard) {
	std::vector<::possumwood::io::json> data;
	std::vector<std::function<void(const dependency_graph::Network&)>> tests;

	//////////////
	// a simple network of nodes
	data.push_back("{\"connections\":[],\"nodes\":{}}"_json);

	data.back()["nodes"]["addition_0"] = possumwood::io::json{
	    {"name", "add"},
	    {"type", "addition"},
	    {"ports", {{"input_1", 2.0}, {"input_2", 4.0}}},
	    {"blind_data", {{"type", unmangledTypeId<possumwood::NodeData>()}, {"value", "test blind data"}}}};

	data.back()["nodes"]["multiplication_0"] = possumwood::io::json{
	    {"name", "mult_0"},
	    {"type", "multiplication"},
	    {"ports", {{"input_2", 5.0}}},
	    {"blind_data", {{"type", unmangledTypeId<possumwood::NodeData>()}, {"value", "test blind data"}}}};

	data.back()["nodes"]["multiplication_1"] = possumwood::io::json{
	    {"name", "mult_1"},
	    {"type", "multiplication"},
	    {"ports", {{"input_1", 0.0}, {"input_2", 0.0}}},
	    {"blind_data", {{"type", unmangledTypeId<possumwood::NodeData>()}, {"value", "test blind data"}}}};

	data.back()["connections"] = possumwood::io::json{{
	    {"out_node", "addition_0"},
	    {"out_port", "output"},
	    {"in_node", "multiplication_0"},
	    {"in_port", "input_1"},
	}};

	tests.push_back([&](const dependency_graph::Network& net) {
		auto m0 = findNode(net, "mult_0");
		BOOST_REQUIRE(m0 != net.nodes().end());

		auto m1 = findNode(net, "mult_1");
		BOOST_REQUIRE(m1 != net.nodes().end());

		auto a0 = findNode(net, "add");
		BOOST_REQUIRE(a0 != net.nodes().end());

		BOOST_CHECK_EQUAL(a0->port(0).get<float>(), 2.0f);
		BOOST_CHECK_EQUAL(a0->port(1).get<float>(), 4.0f);
		BOOST_CHECK_EQUAL(a0->port(2).get<float>(), 6.0f);

		BOOST_CHECK_EQUAL(m0->port(0).get<float>(), 6.0f);
		BOOST_CHECK_EQUAL(m0->port(1).get<float>(), 5.0f);
		BOOST_CHECK_EQUAL(m0->port(2).get<float>(), 30.0f);
	});

	///////////////
	// previous network as a subnetwork (with no inputs or outputs)
	data.push_back(possumwood::io::json());
	data.back()["nodes"]["network_0"] =
	    json{{"name", "test_network"},
	         {"type", "network"},
	         // {"ports", {}},
	         {"blind_data", {{"type", unmangledTypeId<possumwood::NodeData>()}, {"value", "test blind data"}}},
	         {"nodes", data[0]["nodes"]},
	         {"connections", data[0]["connections"]}};
	data.back()["connections"] = "[]"_json;

	tests.push_back([&](const dependency_graph::Network& net) {
		auto s = findNode(net, "test_network");
		BOOST_REQUIRE(s != net.nodes().end());

		BOOST_REQUIRE(s->is<dependency_graph::Network>());
		auto& subnet = s->as<dependency_graph::Network>();

		auto m0 = findNode(subnet, "mult_0");
		BOOST_REQUIRE(m0 != subnet.nodes().end());

		auto m1 = findNode(subnet, "mult_1");
		BOOST_REQUIRE(m1 != subnet.nodes().end());

		auto a0 = findNode(subnet, "add");
		BOOST_REQUIRE(a0 != subnet.nodes().end());

		BOOST_CHECK_EQUAL(a0->port(0).get<float>(), 2.0f);
		BOOST_CHECK_EQUAL(a0->port(1).get<float>(), 4.0f);
		BOOST_CHECK_EQUAL(a0->port(2).get<float>(), 6.0f);

		BOOST_CHECK_EQUAL(m0->port(0).get<float>(), 6.0f);
		BOOST_CHECK_EQUAL(m0->port(1).get<float>(), 5.0f);
		BOOST_CHECK_EQUAL(m0->port(2).get<float>(), 30.0f);
	});

	///////////////
	// previous network as a subnetwork (with inputs and outputs, but not connected)
	//   -> contains unconnected void ports
	data.push_back(data.back());
	data.back()["nodes"]["network_0"]["nodes"]["input_0"] = json{
	    {"name", "input_1"},
	    {"type", "input"},
	    {"blind_data", {{"type", unmangledTypeId<possumwood::NodeData>()}, {"value", "test blind data"}}},
	};
	data.back()["nodes"]["network_0"]["nodes"]["input_1"] = json{
	    {"name", "input_2"},
	    {"type", "input"},
	    {"blind_data", {{"type", unmangledTypeId<possumwood::NodeData>()}, {"value", "test blind data"}}},
	};
	data.back()["nodes"]["network_0"]["nodes"]["output_0"] = json{
	    {"name", "output_1"},
	    {"type", "output"},
	    {"blind_data", {{"type", unmangledTypeId<possumwood::NodeData>()}, {"value", "test blind data"}}},
	};

	tests.push_back([&](const dependency_graph::Network&) {

	});

	////////////
	// previous network, but with connected inputs and output
	data.push_back(data.back());

	// add network port connections
	data.back()["nodes"]["network_0"]["connections"] = json{{
	                                                            {"out_node", "addition_0"},
	                                                            {"out_port", "output"},
	                                                            {"in_node", "output_0"},
	                                                            {"in_port", "data"},
	                                                        },
	                                                        {
	                                                            {"out_node", "addition_0"},
	                                                            {"out_port", "output"},
	                                                            {"in_node", "multiplication_0"},
	                                                            {"in_port", "input_1"},
	                                                        },
	                                                        {
	                                                            {"out_node", "input_0"},
	                                                            {"out_port", "data"},
	                                                            {"in_node", "addition_0"},
	                                                            {"in_port", "input_1"},
	                                                        },
	                                                        {
	                                                            {"out_node", "input_1"},
	                                                            {"out_port", "data"},
	                                                            {"in_node", "addition_0"},
	                                                            {"in_port", "input_2"},
	                                                        }};

	// connected ports will no longer be explicitly listed in addition_0 ports
	data.back()["nodes"]["network_0"]["nodes"]["addition_0"].erase(
	    data.back()["nodes"]["network_0"]["nodes"]["addition_0"].find("ports"));
	// but they will be exposed out on the network
	data.back()["nodes"]["network_0"]["ports"] = json{
	    {"input_1", 7},
	    {"input_2", 8},
	};

	tests.push_back([&](const dependency_graph::Network& net) {
		auto s = findNode(net, "test_network");
		BOOST_REQUIRE(s != net.nodes().end());

		BOOST_REQUIRE(s->is<dependency_graph::Network>());
		auto& subnet = s->as<dependency_graph::Network>();

		auto m0 = findNode(subnet, "mult_0");
		BOOST_REQUIRE(m0 != subnet.nodes().end());

		auto m1 = findNode(subnet, "mult_1");
		BOOST_REQUIRE(m1 != subnet.nodes().end());

		auto a0 = findNode(subnet, "add");
		BOOST_REQUIRE(a0 != subnet.nodes().end());

		BOOST_CHECK_EQUAL(a0->port(0).get<float>(), 7.0f);
		BOOST_CHECK_EQUAL(a0->port(1).get<float>(), 8.0f);
		BOOST_CHECK_EQUAL(a0->port(2).get<float>(), 15.0f);

		BOOST_CHECK_EQUAL(m0->port(0).get<float>(), 15.0f);
		BOOST_CHECK_EQUAL(m0->port(1).get<float>(), 5.0f);
		BOOST_CHECK_EQUAL(m0->port(2).get<float>(), 75.0f);

		BOOST_CHECK_EQUAL(s->port(0).get<float>(), 7.0f);
		BOOST_CHECK_EQUAL(s->port(1).get<float>(), 8.0f);
		BOOST_CHECK_EQUAL(s->port(2).get<float>(), 15.0f);

		// and test pull from outside
		BOOST_REQUIRE_NO_THROW(s->port(0).set(9.0f));

		BOOST_CHECK_EQUAL(s->port(0).get<float>(), 9.0f);
		BOOST_CHECK_EQUAL(s->port(2).get<float>(), 17.0f);
	});

	/////////////////////////////////////////////////////////////////////////////////////////

	// make sure the static handles are initialised
	additionNode();
	multiplicationNode();

	auto test_it = tests.begin();

	for(const auto& pasted_data : data) {
		// make the app "singleton"
		possumwood::AppCore app;

		// test that the undo stack is empty
		BOOST_REQUIRE(app.undoStack().empty());

		// paste the data
		dependency_graph::Selection selection;
		BOOST_REQUIRE_NO_THROW(possumwood::actions::fromJson(app.graph(), selection, pasted_data));

		// check that theres one undo step now
		BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 1u);
		BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

		// check that the pasted data are correct
		{
			::json json;
			BOOST_REQUIRE_NO_THROW(json = possumwood::actions::toJson());
			BOOST_CHECK_EQUAL(json, pasted_data);
		}

		// run the appropriate test
		(*test_it)(app.graph());

		// // perform undo
		// BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

		// // graph should be empty now
		// BOOST_REQUIRE(app.graph().nodes().empty());

		// // check that theres one redo step now
		// BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 0u);
		// BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 1u);

		// // perform redo
		// BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

		// // and check that the result is right
		// {
		// 	::json json;
		// 	BOOST_REQUIRE_NO_THROW(json = possumwood::actions::toJson());
		// 	BOOST_CHECK_EQUAL(json, pasted_data);
		// }

		++test_it;
	}
}
