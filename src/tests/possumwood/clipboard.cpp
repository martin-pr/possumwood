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
						{"in_node", "multiplication_0"},
						{"in_port", "input_1"},
						{"out_node", "addition_0"},
						{"out_port", "output"}
					}
				}
			}
		});

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

		// previous network as a subnetwork (with inputs and outputs)
		data.push_back(data[1]);
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


		dependency_graph::Selection selection;
		BOOST_REQUIRE_NO_THROW(possumwood::actions::paste(possumwood::AppCore::instance().graph(), selection, data));

		::json json;
		BOOST_REQUIRE_NO_THROW(json = possumwood::AppCore::instance().graph());
		BOOST_CHECK_EQUAL(json, data);


}
