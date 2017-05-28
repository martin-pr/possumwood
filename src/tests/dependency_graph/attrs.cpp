#include <boost/test/unit_test.hpp>

#include "common.h"
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/metadata.inl>

using namespace dependency_graph;

// struct TestStruct {
// 	bool operator == (const TestStruct& ts) const { return true; }
// };


BOOST_AUTO_TEST_CASE(input_attr_instantiation) {
	Metadata meta("test_type");
	BOOST_REQUIRE_EQUAL(meta.type(), "test_type");

	{
		// in attribute instantiation -> not initialised
		static InAttr<float> attr;

		BOOST_CHECK(not attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "");
		BOOST_CHECK_EQUAL(attr.category(), Attr::kInput);
		BOOST_CHECK_EQUAL(attr.type(), typeid(float));

		// instantiation
		meta.addAttribute(attr, "first");
		BOOST_CHECK(attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "first");
		BOOST_CHECK_EQUAL(attr.offset(), 0u);
	}

	{
		// in attribute instantiation -> not initialised
		static InAttr<TestStruct> attr;

		BOOST_CHECK(not attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "");
		BOOST_CHECK_EQUAL(attr.category(), Attr::kInput);
		BOOST_CHECK_EQUAL(attr.type(), typeid(TestStruct));

		// instantiation
		meta.addAttribute(attr, "second");
		BOOST_CHECK(attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "second");
		BOOST_CHECK_EQUAL(attr.offset(), 1u);
	}

	{
		// out attribute instantiation -> not initialised
		static OutAttr<float> attr;

		BOOST_CHECK(not attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "");
		BOOST_CHECK_EQUAL(attr.category(), Attr::kOutput);
		BOOST_CHECK_EQUAL(attr.type(), typeid(float));

		// instantiation
		meta.addAttribute(attr, "third");
		BOOST_CHECK(attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "third");
		BOOST_CHECK_EQUAL(attr.offset(), 2u);
	}

	{
		// out attribute instantiation -> not initialised
		static OutAttr<TestStruct> attr;

		BOOST_CHECK(not attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "");
		BOOST_CHECK_EQUAL(attr.category(), Attr::kOutput);
		BOOST_CHECK_EQUAL(attr.type(), typeid(TestStruct));

		// instantiation
		meta.addAttribute(attr, "fourth");
		BOOST_CHECK(attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "fourth");
		BOOST_CHECK_EQUAL(attr.offset(), 3u);
	}

	//////////////
	// testing the result

	BOOST_CHECK_EQUAL(meta.attributeCount(), 4u);

	BOOST_CHECK_EQUAL(meta.attr(0).name(), "first");
	BOOST_CHECK_EQUAL(meta.attr(0).category(), Attr::kInput);
	BOOST_CHECK_EQUAL(meta.attr(0).offset(), 0u);
	BOOST_CHECK_EQUAL(meta.attr(0).type(), typeid(float));

	BOOST_CHECK_EQUAL(meta.attr(1).name(), "second");
	BOOST_CHECK_EQUAL(meta.attr(1).category(), Attr::kInput);
	BOOST_CHECK_EQUAL(meta.attr(1).offset(), 1u);
	BOOST_CHECK_EQUAL(meta.attr(1).type(), typeid(TestStruct));

	BOOST_CHECK_EQUAL(meta.attr(2).name(), "third");
	BOOST_CHECK_EQUAL(meta.attr(2).category(), Attr::kOutput);
	BOOST_CHECK_EQUAL(meta.attr(2).offset(), 2u);
	BOOST_CHECK_EQUAL(meta.attr(2).type(), typeid(float));

	BOOST_CHECK_EQUAL(meta.attr(3).name(), "fourth");
	BOOST_CHECK_EQUAL(meta.attr(3).category(), Attr::kOutput);
	BOOST_CHECK_EQUAL(meta.attr(3).offset(), 3u);
	BOOST_CHECK_EQUAL(meta.attr(3).type(), typeid(TestStruct));
}
