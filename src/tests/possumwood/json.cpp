#include <boost/test/unit_test.hpp>

#include <dependency_graph/graph.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/node.h>
#include <actions/io/graph.h>
#include <dependency_graph/port.inl>
#include <dependency_graph/rtti.h>
#include <dependency_graph/metadata_register.h>

#include "common.h"

using namespace dependency_graph;
using possumwood::io::json;

BOOST_AUTO_TEST_CASE(simple_graph_saving) {
	Graph g;

	// empty serialization
	{
		const json result = "{\"nodes\":{},\"connections\":[]}"_json;

		::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, result);

		Graph g2;
		possumwood::io::adl_serializer<Graph>::from_json(json, g2);

		::json json2;
		BOOST_REQUIRE_NO_THROW(json2 = g2);
		BOOST_CHECK_EQUAL(json2, result);
	}

	// a single node, no connections, no blind data
	NodeBase& a = g.nodes().add(additionNode(), "add");
	{
		const json result(
			{
				{
					"nodes", {
						{"addition_0", {
							{"name", "add"},
							{"type", "addition"},
							{"ports", {
								{"input_1", 2.0},
								{"input_2", 4.0}}},
							{"blind_data", nullptr}
						}}
					}
				},
				{
					"connections", "[]"_json
				}
			});


		a.port(0).set<float>(2.0f);
		a.port(1).set<float>(4.0f);


		::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, result);


		Graph g2;
		possumwood::io::adl_serializer<Graph>::from_json(json, g2);

		::json json2;
		BOOST_REQUIRE_NO_THROW(json2 = g2);
		BOOST_CHECK_EQUAL(json2, result);
	}

	// three nodes, a connection, blind data
	NodeBase& m = g.nodes().add(multiplicationNode(), "mult");
	g.nodes().add(multiplicationNode(), "mult");
	{
		const json result(
			{
				{
					"nodes", {
						{"addition_0", {
							{"name", "add"},
							{"type", "addition"},
							{"ports", {
								{"input_1", 2.0},
								{"input_2", 4.0}
							}},
							{"blind_data", nullptr}
						}},
						{"multiplication_0", {
							{"name", "mult"},
							{"type", "multiplication"},
							{"ports", {
								{"input_2", 5.0}
							}},
							{"blind_data", {
								{"type", unmangledTypeId<std::string>()},
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
							{"blind_data", nullptr}
						}}
					}
				},
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
			}
		);


		m.port(0).set<float>(3.0f);
		m.port(1).set<float>(5.0f);

		a.port(2).connect(m.port(0));

		m.setBlindData<std::string>("test blind data");


		::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, result);


		Graph g2;
		possumwood::io::adl_serializer<Graph>::from_json(json, g2);

		::json json2;
		BOOST_REQUIRE_NO_THROW(json2 = g2);
		BOOST_CHECK_EQUAL(json2, result);
	}
}


BOOST_AUTO_TEST_CASE(nested_graph_saving) {
	Graph g;

	Network* net = nullptr;

	// empty network serialization
	{
		// add an empty network
		{
			MetadataHandle networkMeta = MetadataRegister::singleton()["network"];
			NodeBase& base = g.nodes().add(networkMeta, "test_network");

			BOOST_REQUIRE(base.is<Network>());
			net = &(base.as<Network>());
		}

		const json result(
			{
				{
					"nodes", {
						{"network_0", {
							{"name", "test_network"},
							{"type", "network"},
							// {"ports", {}},
							{"blind_data", nullptr},
							{"nodes", "{}"_json},
							{"connections", "[]"_json}
						}}
					},
				},
				{
					"connections", "[]"_json
				}
			}
		);

		::json json;

		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, result);

		Graph g2;
		possumwood::io::adl_serializer<Graph>::from_json(json, g2);

		::json json2;
		BOOST_REQUIRE_NO_THROW(json2 = g2);
		BOOST_CHECK_EQUAL(json2, result);
	}

	// a single node, no connections, no blind data
	NodeBase& a = net->nodes().add(additionNode(), "add");
	{
		const json result(
			{
				{
					"nodes", {
						{"network_0", {
							{"name", "test_network"},
							{"type", "network"},
							// {"ports", {}},
							{"blind_data", nullptr},
							{"nodes", {
								{"addition_0", {
									{"name", "add"},
									{"type", "addition"},
									{"ports", {
										{"input_1", 2.0},
										{"input_2", 4.0}}},
									{"blind_data", nullptr}
								}}
							}},
							{"connections", "[]"_json}
						}}
					},
				},
				{
					"connections", "[]"_json
				}
			}
		);

		a.port(0).set<float>(2.0f);
		a.port(1).set<float>(4.0f);

		// test saving
		::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, result);

		// and test that it can be loaded again
		Graph g2;
		possumwood::io::adl_serializer<Graph>::from_json(json, g2);

		::json json2;
		BOOST_REQUIRE_NO_THROW(json2 = g2);
		BOOST_CHECK_EQUAL(json2, result);
	}

	// three nodes, a connection, blind data
	NodeBase& m = net->nodes().add(multiplicationNode(), "mult");
	net->nodes().add(multiplicationNode(), "mult");
	{
		const json result(
			{
				{
					"nodes", {
						{"network_0", {
							{"name", "test_network"},
							{"type", "network"},
							// {"ports", {}},
							{"blind_data", nullptr},
							{"nodes", {
								{"addition_0", {
									{"name", "add"},
									{"type", "addition"},
									{"ports", {
										{"input_1", 2.0},
										{"input_2", 4.0}
									}},
									{"blind_data", nullptr}
								}},
								{"multiplication_0", {
									{"name", "mult"},
									{"type", "multiplication"},
									{"ports", {
										{"input_2", 5.0}
									}},
									{"blind_data", {
										{"type", unmangledTypeId<std::string>()},
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
									{"blind_data", nullptr}
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


		m.port(0).set<float>(3.0f);
		m.port(1).set<float>(5.0f);

		a.port(2).connect(m.port(0));

		m.setBlindData<std::string>("test blind data");


		::json json;
		BOOST_REQUIRE_NO_THROW(json = g);
		BOOST_CHECK_EQUAL(json, result);


		Graph g2;
		possumwood::io::adl_serializer<Graph>::from_json(json, g2);

		::json json2;
		BOOST_REQUIRE_NO_THROW(json2 = g2);
		BOOST_CHECK_EQUAL(json2, result);
	}
}