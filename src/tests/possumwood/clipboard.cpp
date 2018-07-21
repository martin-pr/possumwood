#include <boost/test/unit_test.hpp>

#include <vector>

#include <dependency_graph/graph.h>
#include <dependency_graph/rtti.h>

#include <actions/io/graph.h>
#include <actions/actions.h>

#include <possumwood_sdk/app.h>

#include "common.h"

using namespace dependency_graph;
using possumwood::io::json;

namespace {

const std::vector<::possumwood::io::json> testData() {
	static std::vector<::possumwood::io::json> data;

	if(data.empty()) {
		////////////
		// a simple network of nodes
		data.push_back(possumwood::io::json {
			{"nodes", {
				{"addition_0", {
					{"name", "add"},
					{"type", "addition"},
					{"ports", {
						{"input_1", 2.0},
						{"input_2", 4.0}
					}},
					{"blind_data", {
						{"type", unmangledTypeId<possumwood::NodeData>()},
						{"value", "test blind data"}
					}}
				}},
				{"multiplication_0", {
					{"name", "mult"},
					{"type", "multiplication"},
					{"ports", {
						{"input_2", 5.0}
					}},
					{"blind_data", {
						{"type", unmangledTypeId<possumwood::NodeData>()},
						{"value", "test blind data"}
					}}
				}},
				{"multiplication_1", {
					{"name", "mult"},
					{"type", "multiplication"},
					{"ports", {
						{"input_1", 0.0},
						{"input_2", 0.0}
					}},
					{"blind_data", {
						{"type", unmangledTypeId<possumwood::NodeData>()},
						{"value", "test blind data"}
					}}
				}}
			}},
			{
				"connections", {
					{
						{"out_node", "addition_0"},
						{"out_port", "output"},
						{"in_node", "multiplication_0"},
						{"in_port", "input_1"},
					}
				}
			}
		});

		///////////////
		// previous network as a subnetwork (with no inputs or outputs)
		data.push_back(possumwood::io::json());
		data.back()["nodes"]["network_0"] = json{
			{"name", "test_network"},
			{"type", "network"},
			// {"ports", {}},
			{"blind_data", {
				{"type", unmangledTypeId<possumwood::NodeData>()},
				{"value", "test blind data"}
			}},
			{"nodes", data[0]["nodes"]},
			{"connections", data[0]["connections"]}
		};
		data.back()["connections"] = "[]"_json;

		///////////////
		// previous network as a subnetwork (with inputs and outputs, but not connected)
		data.push_back(data.back());
		data.back()["nodes"]["network_0"]["nodes"]["input_0"] = json{
			{"name", "input_1"},
			{"type", "input"},
			{"blind_data", {
				{"type", unmangledTypeId<possumwood::NodeData>()},
				{"value", "test blind data"}
			}},
		};
		data.back()["nodes"]["network_0"]["nodes"]["input_1"] = json{
			{"name", "input_2"},
			{"type", "input"},
			{"blind_data", {
				{"type", unmangledTypeId<possumwood::NodeData>()},
				{"value", "test blind data"}
			}},
		};
		data.back()["nodes"]["network_0"]["nodes"]["output_0"] = json{
			{"name", "output_1"},
			{"type", "output"},
			{"blind_data", {
				{"type", unmangledTypeId<possumwood::NodeData>()},
				{"value", "test blind data"}
			}},
		};

		////////////
		// previous network, but with connected inputs and output
		data.push_back(data.back());

		// add network port connections
		data.back()["nodes"]["network_0"]["connections"] = json {
			{
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
			}
		};

		// connected ports will no longer be explicitly listed in addition_0 ports
		data.back()["nodes"]["network_0"]["nodes"]["addition_0"].erase(data.back()["nodes"]["network_0"]["nodes"]["addition_0"].find("ports"));
		// but they will be exposed out on the network
		data.back()["nodes"]["network_0"]["ports"] = json {
			{"input_1", 0},
			{"input_2", 0},
		};
	}

	return data;
}

}

BOOST_AUTO_TEST_CASE(clipboard) {
	// make sure the static handles are initialised
	additionNode();
	multiplicationNode();

	for(auto& data : testData()) {
		// make the app "singleton"
		possumwood::AppCore app;

		// test that the undo stack is empty
		BOOST_REQUIRE(app.undoStack().empty());

		// paste the data
		dependency_graph::Selection selection;
		BOOST_REQUIRE_NO_THROW(possumwood::actions::paste(app.graph(), selection, data));

		// check that theres one undo step now
		BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 1u);
		BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

		// check that the pasted data are correct
		{
			::json json;
			BOOST_REQUIRE_NO_THROW(json = app.graph());
			BOOST_CHECK_EQUAL(json, data);
		}

		// perform undo
		BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

		// graph should be empty now
		BOOST_REQUIRE(app.graph().nodes().empty());

		// check that theres one redo step now
		BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 0u);
		BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 1u);

		// perform redo
		BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

		// and check that the result is right
		{
			::json json;
			BOOST_REQUIRE_NO_THROW(json = app.graph());
			BOOST_CHECK_EQUAL(json, data);
		}
	}
}
