#include <dependency_graph/graph.h>
#include <dependency_graph/metadata_register.h>
#include <dependency_graph/node.h>

#include <boost/test/unit_test.hpp>
#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/values.inl>

#include "common.h"

using namespace dependency_graph;

namespace {

const dependency_graph::MetadataHandle& multiConnect() {
	static std::unique_ptr<MetadataHandle> s_handle;

	if(s_handle == nullptr) {
		std::unique_ptr<Metadata> meta(new Metadata("multi_input"));

		// create attributes
		InAttr<float> floatInput;
		meta->addAttribute(floatInput, "float_input");

		OutAttr<float> floatOutput;
		meta->addAttribute(floatOutput, "float_output");

		meta->addInfluence(floatInput, floatOutput);

		InAttr<int> intInput;
		meta->addAttribute(intInput, "int_input");

		OutAttr<int> intOutput;
		meta->addAttribute(intOutput, "int_output");

		meta->addInfluence(intInput, intOutput);

		// an output with the number of operations executed
		OutAttr<unsigned> counterOutput;
		meta->addAttribute(counterOutput, "counter_output");
		meta->addInfluence(floatInput, counterOutput);
		meta->addInfluence(intInput, counterOutput);

		meta->setCompute([=](dependency_graph::Values& vals) {
			unsigned counter = 0;

			if(vals.isDirty(floatOutput)) {
				vals.set(floatOutput, vals.get(floatInput));
				++counter;
			}

			if(vals.isDirty(intOutput)) {
				vals.set(intOutput, vals.get(intInput));
				++counter;
			}

			vals.set(counterOutput, counter);

			return dependency_graph::State();
		});

		s_handle = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(meta)));

		dependency_graph::MetadataRegister::singleton().add(*s_handle);
	}

	return *s_handle;
}

bool checkDirtyPorts(const NodeBase& n, const std::set<size_t>& dirtyPorts) {
	bool result = true;

	for(size_t pi = 0; pi < n.portCount(); ++pi) {
		BOOST_CHECK_EQUAL(n.port(pi).isDirty(), dirtyPorts.find(pi) != dirtyPorts.end());

		if(n.port(pi).isDirty() != (dirtyPorts.find(pi) != dirtyPorts.end()))
			result = false;
	}

	return result;
}

}  // namespace

// need to add dirty querying on dependency_graph::Values, to allow partial evaluation
//   -> allows to do a multiple-output multiple-input complex node (or NETWORK)
//   + TESTS

BOOST_AUTO_TEST_CASE(multiple_port_compute) {
	Graph g;

	// make a node to play with
	const MetadataHandle& handle = multiConnect();
	NodeBase& node = g.nodes().add(handle, "node");

	// after creation, outputs should be dirty
	BOOST_CHECK(checkDirtyPorts(node, {1, 3, 4}));

	// pull on the float output
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 0.0f);

	// compute() got called once, but both outputs were dirty and got evaluated - nothing is dirty, and count is 2
	BOOST_CHECK(checkDirtyPorts(node, {}));
	BOOST_CHECK_EQUAL(node.port(4).get<unsigned>(), 2u);

	// lets set a new value on the int input
	BOOST_REQUIRE_NO_THROW(node.port(2).set<int>(1));

	// now we should have the int and counter outputs dirty
	BOOST_CHECK(checkDirtyPorts(node, {3, 4}));

	// pulling on either of them will trigger evaluation
	BOOST_CHECK_EQUAL(node.port(3).get<int>(), 1);
	// but the counter should be just 1 - float did not get recomputed
	BOOST_CHECK_EQUAL(node.port(4).get<unsigned>(), 1u);
	// after which nothing is dirty
	BOOST_CHECK(checkDirtyPorts(node, {}));

	// lets set a new value on both inputs
	BOOST_REQUIRE_NO_THROW(node.port(0).set<float>(2.0f));
	BOOST_REQUIRE_NO_THROW(node.port(2).set<int>(2));

	// all outputs will be dirty now
	BOOST_CHECK(checkDirtyPorts(node, {1, 3, 4}));

	// pull on one
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 2.0f);
	// nothing is dirty now
	BOOST_CHECK(checkDirtyPorts(node, {}));
	BOOST_CHECK_EQUAL(node.port(3).get<int>(), 2);
	// and 2 computations happened
	BOOST_CHECK_EQUAL(node.port(4).get<unsigned>(), 2);
}
