#include <boost/test/unit_test.hpp>

#include <dependency_graph/graph.h>
#include <dependency_graph/metadata_register.h>

#include <actions/actions.h>
#include <actions/app.h>

#include "common.h"

using namespace dependency_graph;

namespace {

dependency_graph::NodeBase& findNode(dependency_graph::Network& net, const std::string& name) {
	for(auto& n : net.nodes())
		if(n.name() == name)
			return n;

	BOOST_REQUIRE(false && "Node not found, fail");
	throw;
}

}

BOOST_AUTO_TEST_CASE(nested) {
	possumwood::AppCore app;

	const MetadataHandle& addition = additionNode();
	const MetadataHandle& multiplication = multiplicationNode();

	const MetadataHandle& network = MetadataRegister::singleton()["network"];
	const MetadataHandle& input = MetadataRegister::singleton()["input"];
	const MetadataHandle& output = MetadataRegister::singleton()["output"];

	////////////////////////////
	// build a simple graph for ((a + b) * c) + ((a + b) * d)
	// with each bracketed expression in a sub-network

	// (((a + b) * c) + ((a + b) * d))
	// ││└net4─┘    │   │└net5─┘    ││
	// │└── net2 ───┘   └─── net3 ──┘│
	// └─────────── net1 ────────────┘

	BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(app.graph(), network, "net1", possumwood::NodeData()));
	Network& net1 = dynamic_cast<Network&>(findNode(app.graph(), "net1"));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net1, network, "net2", possumwood::NodeData()));
	Network& net2 = dynamic_cast<Network&>(findNode(net1, "net2"));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net1, network, "net3", possumwood::NodeData()));
	Network& net3 = dynamic_cast<Network&>(findNode(net1, "net3"));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net2, network, "net4", possumwood::NodeData()));
	Network& net4 = dynamic_cast<Network&>(findNode(net2, "net4"));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net3, network, "net5", possumwood::NodeData()));
	Network& net5 = dynamic_cast<Network&>(findNode(net3, "net5"));

	{
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net5, addition, "add5", possumwood::NodeData()));
		NodeBase& net5_add5 = findNode(net5, "add5");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net5, input, "net5_in1", possumwood::NodeData()));
		NodeBase& net5_in1 = findNode(net5, "net5_in1");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net5, input, "net5_in2", possumwood::NodeData()));
		NodeBase& net5_in2 = findNode(net5, "net5_in2");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net5, output, "net5_out", possumwood::NodeData()));
		NodeBase& net5_out = findNode(net5, "net5_out");

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net5_in1.port(0), net5_add5.port(0)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net5_in2.port(0), net5_add5.port(1)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net5_add5.port(2), net5_out.port(0)));
	}

	{
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net4, addition, "add4", possumwood::NodeData()));
		NodeBase& add4 = findNode(net4, "add4");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net4, input, "net4_in1", possumwood::NodeData()));
		NodeBase& in1 = findNode(net4, "net4_in1");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net4, input, "net4_in2", possumwood::NodeData()));
		NodeBase& in2 = findNode(net4, "net4_in2");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net4, output, "net4_out", possumwood::NodeData()));
		NodeBase& out = findNode(net4, "net4_out");

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(in1.port(0), add4.port(0)));
		BOOST_CHECK(net4.port(0).isLinked());

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(in2.port(0), add4.port(1)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(add4.port(2), out.port(0)));
		BOOST_CHECK(net4.port(0).isLinked());
	}

	{
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net3, multiplication, "mult3", possumwood::NodeData()));
		NodeBase& net3_mult3 = findNode(net3, "mult3");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net3, input, "net3_in1", possumwood::NodeData()));
		NodeBase& net3_in1 = findNode(net3, "net3_in1");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net3, input, "net3_in2", possumwood::NodeData()));
		NodeBase& net3_in2 = findNode(net3, "net3_in2");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net3, input, "net3_in3", possumwood::NodeData()));
		NodeBase& net3_in3 = findNode(net3, "net3_in3");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net3, output, "net3_out", possumwood::NodeData()));
		NodeBase& net3_out = findNode(net3, "net3_out");

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net3_in1.port(0), net5.port(0)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net3_in2.port(0), net5.port(1)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net3_in3.port(0), net3_mult3.port(0)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net5.port(2), net3_mult3.port(1)));

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net3_mult3.port(2), net3_out.port(0)));
	}

	{
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net2, multiplication, "mult2", possumwood::NodeData()));
		NodeBase& mult2 = findNode(net2, "mult2");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net2, input, "net2_in1", possumwood::NodeData()));
		NodeBase& net2_in1 = findNode(net2, "net2_in1");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net2, input, "net2_in2", possumwood::NodeData()));
		NodeBase& net2_in2 = findNode(net2, "net2_in2");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net2, input, "net2_in3", possumwood::NodeData()));
		NodeBase& net2_in3 = findNode(net2, "net2_in3");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net2, output, "net2_out", possumwood::NodeData()));
		NodeBase& net2_out = findNode(net2, "net2_out");

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net2_in1.port(0), net4.port(0)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net2_in2.port(0), net4.port(1)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net2_in3.port(0), mult2.port(0)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net4.port(2), mult2.port(1)));

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(mult2.port(2), net2_out.port(0)));
	}

	{
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net1, addition, "add1", possumwood::NodeData()));
		NodeBase& add1 = findNode(net1, "add1");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net1, input, "net1_in1", possumwood::NodeData()));
		NodeBase& in1 = findNode(net1, "net1_in1");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net1, input, "net1_in2", possumwood::NodeData()));
		NodeBase& in2 = findNode(net1, "net1_in2");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net1, input, "net1_in3", possumwood::NodeData()));
		NodeBase& in3 = findNode(net1, "net1_in3");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net1, input, "net1_in4", possumwood::NodeData()));
		NodeBase& in4 = findNode(net1, "net1_in4");
		BOOST_REQUIRE_NO_THROW(possumwood::actions::createNode(net1, output, "net1_out", possumwood::NodeData()));
		NodeBase& out = findNode(net1, "net1_out");

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(in1.port(0), net2.port(0)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(in1.port(0), net3.port(0)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(in2.port(0), net2.port(1)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(in2.port(0), net3.port(1)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(in3.port(0), net2.port(2)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(in4.port(0), net3.port(2)));

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net2.port(3), add1.port(0)));
		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(net3.port(3), add1.port(1)));

		BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(add1.port(2), out.port(0)));
	}

	// just after creation, the output should be 0
	BOOST_CHECK_EQUAL(net1.port(4).get<float>(), 0.0f);


	// setting a to 1 should give the value of net4 and net5 output to 1
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(0), 1.0f));

	BOOST_CHECK_EQUAL(net1.port(0).get<float>(), 1.0f);
	BOOST_CHECK(net1.port(0).isLinked());

	BOOST_CHECK_EQUAL(net2.port(0).get<float>(), 1.0f);
	BOOST_CHECK(net2.port(0).isLinked());

	BOOST_CHECK_EQUAL(net4.port(0).get<float>(), 1.0f);
	BOOST_CHECK(net4.port(0).isLinked());

	BOOST_CHECK_EQUAL(net4.port(0).get<float>(), 1.0f);
	BOOST_CHECK_EQUAL(net5.port(0).get<float>(), 1.0f);

	BOOST_CHECK_EQUAL(net1.port(4).get<float>(), 0.0f);


	// setting A to 1 and C to 1 should lead to 1.0f
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(0), 1.0f));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(2), 1.0f));

	BOOST_CHECK_EQUAL(net1.port(4).get<float>(), 1.0f);


	// setting all to 1 should lead to 4.0f
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(0), 1.0f));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(1), 1.0f));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(2), 1.0f));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(3), 1.0f));

	BOOST_CHECK_EQUAL(net1.port(4).get<float>(), 4.0f);


	// setting the inputs to 1,2,3,4 should lead to 21
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(0), 1.0f));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(1), 2.0f));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(2), 3.0f));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(net1.port(3), 4.0f));

	BOOST_CHECK_EQUAL(net1.port(4).get<float>(), 21.0f);
}
