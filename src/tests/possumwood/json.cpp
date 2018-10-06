#include <boost/test/unit_test.hpp>

#include <dependency_graph/graph.h>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/nodes.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/rtti.h>
#include <dependency_graph/metadata_register.h>

#include <actions/actions.h>

#include <possumwood_sdk/app.h>

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

}

BOOST_AUTO_TEST_CASE(simple_graph_saving) {
	possumwood::App app;

	// make sure the static handles are initialised
	additionNode();
	multiplicationNode();

	// empty serialization
	{
		const json result = "{\"nodes\":{},\"connections\":[]}"_json;

		{
			assert(!app.graph().hasParentNetwork());

			::json json;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json, false));

			BOOST_CHECK_EQUAL(json, result);
		}

		{
			BOOST_REQUIRE_NO_THROW(app.loadFile(result));

			::json json2;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json2, false));
			BOOST_CHECK_EQUAL(json2, result);
		}
	}

	// a single node, no connections, no blind data
	{
		NodeBase& a = app.graph().nodes().add(additionNode(), "add");

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

		{
			::json json;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json, false));
			BOOST_CHECK_EQUAL(json, result);


			BOOST_REQUIRE_NO_THROW(app.loadFile(json));
		}

		{
			::json json2;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json2, false));
			BOOST_CHECK_EQUAL(json2, result);
		}
	}

	// three nodes, a connection, blind data
	{
		NodeBase& m = app.graph().nodes().add(multiplicationNode(), "mult");
		app.graph().nodes().add(multiplicationNode(), "mult");

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

		findNode("add").port(2).connect(m.port(0));

		m.setBlindData<std::string>("test blind data");


		{
			::json json;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json, false));
			BOOST_CHECK_EQUAL(json, result);


			BOOST_REQUIRE_NO_THROW(app.loadFile(json));
		}

		{
			::json json2;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json2, false));
			BOOST_CHECK_EQUAL(json2, result);
		}
	}
}


BOOST_AUTO_TEST_CASE(nested_graph_saving) {
	possumwood::App app;

	// empty network serialization
	{
		// add an empty network
		{
			MetadataHandle networkMeta = MetadataRegister::singleton()["network"];
			NodeBase& base = app.graph().nodes().add(networkMeta, "test_network");

			BOOST_REQUIRE(base.is<Network>());
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

		{
			::json json;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json, false));
			BOOST_CHECK_EQUAL(json, result);


			BOOST_REQUIRE_NO_THROW(app.loadFile(json));
		}

		{
			::json json2;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json2, false));
			BOOST_CHECK_EQUAL(json2, result);
		}
	}

	// a single node, no connections, no blind data
	{
		NodeBase& a = findNode("test_network").as<dependency_graph::Network>().
			nodes().add(additionNode(), "add");

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

		{
			::json json;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json, false));
			BOOST_CHECK_EQUAL(json, result);


			BOOST_REQUIRE_NO_THROW(app.loadFile(json));
		}

		{
			::json json2;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json2, false));
			BOOST_CHECK_EQUAL(json2, result);
		}
	}

	// three nodes, a connection, blind data
	{
		auto& net = findNode("test_network").as<dependency_graph::Network>();

		NodeBase& m = net.nodes().add(multiplicationNode(), "mult");
		net.nodes().add(multiplicationNode(), "mult");

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

		findNode(net, "add").port(2).connect(m.port(0));

		m.setBlindData<std::string>("test blind data");


		{
			::json json;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json, false));
			BOOST_CHECK_EQUAL(json, result);


			BOOST_REQUIRE_NO_THROW(app.loadFile(json));
		}

		{
			::json json2;
			BOOST_REQUIRE_NO_THROW(app.saveFile(json2, false));
			BOOST_CHECK_EQUAL(json2, result);
		}
	}
}
