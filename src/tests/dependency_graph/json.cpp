#include <boost/test/unit_test.hpp>

#include <dependency_graph/graph.h>
#include <dependency_graph/node.inl>
#include <dependency_graph/io/graph.h>

#include "common.h"

using namespace dependency_graph;

BOOST_AUTO_TEST_CASE(json_io) {
	Graph g;

	// empty serialization
	{
		io::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, "{\"nodes\":[],\"connections\":[]}"_json);
	}

	// a single node, no connections, no blind data
	Node& a = g.nodes().add(additionNode(), "add");
	{
		a.port(0).set<float>(2.0f);
		a.port(1).set<float>(4.0f);

		io::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, io::json(
			{
				{
					"nodes", {
						{
							{"name", "add"},
							{"type", "addition"},
							{"ports", {2.0, 4.0, nullptr}},
							{"blind_data", nullptr}
						}
					}
				},
				{
					"connections", "[]"_json
				}
			})
		);
	}

	// two nodes, a connection, blind data
	Node& m = g.nodes().add(multiplicationNode(), "mult");
	{
		m.port(0).set<float>(3.0f);
		m.port(1).set<float>(5.0f);

		a.port(2).connect(m.port(0));

		m.setBlindData<std::string>("test blind data");


		io::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, io::json(
			{
				{
					"nodes", {
						{
							{"name", "add"},
							{"type", "addition"},
							{"ports", {2.0, 4.0, nullptr}},
							{"blind_data", nullptr}
						},
						{
							{"name", "mult"},
							{"type", "multiplication"},
							{"ports", {nullptr, 5.0, nullptr}},
							{"blind_data", {
								{"type", typeid(std::string).name()},
								{"value", "test blind data"}
							}}
						}
					}
				},
				{
					"connections", {
						{
							{"in_node", 1},
							{"in_port", 0},
							{"out_node", 0},
							{"out_port", 2}
						}
					}
				}
			})
		);
	}
}
