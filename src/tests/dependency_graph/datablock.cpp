#include <boost/test/unit_test.hpp>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/metadata.inl>

#include "common.h"

namespace dependency_graph {

// a dummy Node implementation - this is what a node will use
class Node {
  public:
	template <typename T>
	static void setInput(const InAttr<T>& attr, Datablock& data, typename std::remove_reference<T>::type&& value) {
		data.set(attr.offset(), std::move(value));
	}

	template <typename T>
	static const T& getOutput(const OutAttr<T>& attr, const Datablock& data) {
		return data.get<T>(attr.offset());
	}
};

}  // namespace dependency_graph

using namespace dependency_graph;

////////

BOOST_AUTO_TEST_CASE(datablock_read_and_write) {
	// construct the metadata
	std::unique_ptr<Metadata> ptr(new Metadata("test_type"));

	InAttr<float> in1;
	ptr->addAttribute(in1, "input_1");

	InAttr<TestStruct> in2;
	ptr->addAttribute(in2, "input_2");

	InAttr<NoncopyableStruct> in3;
	ptr->addAttribute(in3, "input_3");

	OutAttr<float> out1;
	ptr->addAttribute(out1, "output_1");

	OutAttr<TestStruct> out2;
	ptr->addAttribute(out2, "output_2");

	OutAttr<NoncopyableStruct> out3;
	ptr->addAttribute(out3, "output_3");

	std::unique_ptr<MetadataHandle> meta = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(ptr)));

	// check that the TestStructs work as expected
	{
		TestStruct t1;
		TestStruct t2;
		BOOST_REQUIRE(t1 != t2);

		t2 = t1;
		BOOST_REQUIRE_EQUAL(t1, t2);
	}

	// get and set a few values
	Datablock data(*meta);

	// PODs
	{
		BOOST_CHECK(data.get<float>(in1.offset()) != 33.0f);
		BOOST_REQUIRE_NO_THROW(Node::setInput(in1, data, 33.0f));
		BOOST_CHECK_EQUAL(data.get<float>(in1.offset()), 33.0f);

		BOOST_CHECK(Node::getOutput(out1, data) != 22.0f);
		BOOST_REQUIRE_NO_THROW(data.set(out1.offset(), 22.0f));
		BOOST_CHECK_EQUAL(Node::getOutput(out1, data), 22.0f);
	}

	// copyable types
	{
		TestStruct test1;
		const unsigned val1 = test1.id;

		BOOST_CHECK(data.get<TestStruct>(in2.offset()).id != val1);
		BOOST_REQUIRE_NO_THROW(Node::setInput(in2, data, std::move(test1)));
		BOOST_CHECK_EQUAL(data.get<TestStruct>(in2.offset()).id, val1);

		TestStruct test2;
		const unsigned val2 = test2.id;

		BOOST_CHECK(Node::getOutput(out2, data).id != val2);
		BOOST_REQUIRE_NO_THROW(data.set(out2.offset(), std::move(test2)));
		BOOST_CHECK_EQUAL(Node::getOutput(out2, data).id, val2);
	}

	// and non-copyable types
	{
		NoncopyableStruct test1;
		const unsigned val1 = test1.id;

		BOOST_CHECK(data.get<NoncopyableStruct>(in3.offset()).id != val1);
		BOOST_REQUIRE_NO_THROW(Node::setInput(in3, data, std::move(test1)));
		BOOST_CHECK_EQUAL(data.get<NoncopyableStruct>(in3.offset()).id, val1);

		NoncopyableStruct test2;
		const unsigned val2 = test2.id;

		BOOST_CHECK(Node::getOutput(out3, data).id != val2);
		BOOST_REQUIRE_NO_THROW(data.set(out3.offset(), std::move(test2)));
		BOOST_CHECK_EQUAL(Node::getOutput(out3, data).id, val2);
	}
}
