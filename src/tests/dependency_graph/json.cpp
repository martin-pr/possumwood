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
		const io::json result = "{\"nodes\":[],\"connections\":[]}"_json;

		io::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, result);

		Graph g2;
		dependency_graph::io::adl_serializer<Graph>::from_json(json, g2);

		io::json json2;
		BOOST_REQUIRE_NO_THROW(json2 = g2);
		BOOST_CHECK_EQUAL(json2, result);
	}

	// a single node, no connections, no blind data
	Node& a = g.nodes().add(additionNode(), "add");
	{
		const io::json result(
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
			});


		a.port(0).set<float>(2.0f);
		a.port(1).set<float>(4.0f);


		io::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, result);


		Graph g2;
		dependency_graph::io::adl_serializer<Graph>::from_json(json, g2);

		io::json json2;
		BOOST_REQUIRE_NO_THROW(json2 = g2);
		BOOST_CHECK_EQUAL(json2, result);
	}

	// two nodes, a connection, blind data
	Node& m = g.nodes().add(multiplicationNode(), "mult");
	{
		const io::json result(
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
			}
		);


		m.port(0).set<float>(3.0f);
		m.port(1).set<float>(5.0f);

		a.port(2).connect(m.port(0));

		m.setBlindData<std::string>("test blind data");


		io::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, result);


		Graph g2;
		dependency_graph::io::adl_serializer<Graph>::from_json(json, g2);

		io::json json2;
		BOOST_REQUIRE_NO_THROW(json2 = g2);
		BOOST_CHECK_EQUAL(json2, result);
	}
}
