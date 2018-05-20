#include <boost/test/unit_test.hpp>

#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/graph.h>
#include <dependency_graph/values.inl>

using namespace dependency_graph;

BOOST_AUTO_TEST_CASE(meta_simple_node) {
	// first, make an empty metadata instance
	std::unique_ptr<Metadata> firstMeta(new Metadata("simple_1"));

	// add two attrs
	InAttr<float> input;
	firstMeta->addAttribute(input, "in");

	OutAttr<float> output;
	firstMeta->addAttribute(output, "out");

	firstMeta->addInfluence(input, output);

	firstMeta->setCompute([&](Values& vals) {
		vals.set(output, vals.get(input));

		return dependency_graph::State();
	});

	// make a handle out of this, and create a node
	MetadataHandle firstHandle(std::move(firstMeta));

	// and now instantiate the node
	{
		Graph g;
		g.nodes().add(firstHandle, "first");

		// test that the attributes are right

		// and test the "pull"
	}

	// make new metadata

	// change the metadata of the node

	// callback - check that it fired

	// try to pull on the value
}
