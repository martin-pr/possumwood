#include <boost/test/unit_test.hpp>

#include "common.h"
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/metadata.inl>

namespace dependency_graph {

// a dummy Node implementation - this is what a node will use
class Node {
	public:
		template<typename T>
		static void setInput(const InAttr<T>& attr, Datablock& data, const T& value) {
			data.set(attr.offset(), value);
		}

		template<typename T>
		static const T& getOutput(const OutAttr<T>& attr, const Datablock& data) {
			return data.get<T>(attr.offset());
		}
};

}

using namespace dependency_graph;

////////

BOOST_AUTO_TEST_CASE(datablock_read_and_write) {
	// construct the metadata
	Metadata meta("test_type");

	InAttr<float> in1;
	meta.addAttribute(in1, "input_1");

	InAttr<TestStruct> in2;
	meta.addAttribute(in2, "input_2");

	OutAttr<float> out1;
	meta.addAttribute(out1, "output_1");

	OutAttr<TestStruct> out2;
	meta.addAttribute(out2, "output_2");


	// check that the TestStructs work as expected
	{
		TestStruct t1;
		TestStruct t2;
		BOOST_REQUIRE(t1 != t2);

		t2 = t1;
		BOOST_REQUIRE_EQUAL(t1, t2);
	}


	// get and set a few values
	Datablock data(meta);

	{
		BOOST_CHECK(data.get<float>(in1.offset()) != 33.0f);
		BOOST_REQUIRE_NO_THROW(Node::setInput(in1, data, 33.0f));
		BOOST_CHECK_EQUAL(data.get<float>(in1.offset()), 33.0f);

		TestStruct test1;
		BOOST_CHECK(data.get<TestStruct>(in2.offset()) != test1);
		BOOST_REQUIRE_NO_THROW(Node::setInput(in2, data, test1));
		BOOST_CHECK_EQUAL(data.get<TestStruct>(in2.offset()), test1);

		BOOST_CHECK(Node::getOutput(out1, data) != 22.0f);
		BOOST_REQUIRE_NO_THROW(data.set(out1.offset(), 22.0f));
		BOOST_CHECK_EQUAL(Node::getOutput(out1, data), 22.0f);

		TestStruct test2;
		BOOST_CHECK(Node::getOutput(out2, data) != test2);
		BOOST_REQUIRE_NO_THROW(data.set(out2.offset(), test2));
		BOOST_CHECK_EQUAL(Node::getOutput(out2, data), test2);
	}
}
