#include <boost/test/unit_test.hpp>

#include <dependency_graph/graph.h>
#include <dependency_graph/rtti.h>

#include <actions/io/graph.h>
#include <actions/actions.h>

#include <possumwood_sdk/app.h>

#include "common.h"

using namespace dependency_graph;
using possumwood::io::json;

BOOST_AUTO_TEST_CASE(clipboard) {
	// make the app "singleton"
	possumwood::AppCore app;

	// make sure the static handles are initialised
	additionNode();
	multiplicationNode();

	// the json data to be pasted
	const json pasted_data (
		{
			{
				"nodes", {
					{"network_0", {
						{"name", "test_network"},
						{"type", "network"},
						// {"ports", {}},
						{"blind_data", {
							{"type", unmangledTypeId<possumwood::NodeData>()},
							{"value", "test blind data"}
						}},
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
					}}
				},
			},
			{
				"connections", "[]"_json
			}
		}
	);

	dependency_graph::Selection selection;
	BOOST_REQUIRE_NO_THROW(possumwood::actions::paste(possumwood::AppCore::instance().graph(), selection, pasted_data));

	::json json;
	BOOST_REQUIRE_NO_THROW(json = possumwood::AppCore::instance().graph());
	BOOST_CHECK_EQUAL(json, pasted_data);
}
