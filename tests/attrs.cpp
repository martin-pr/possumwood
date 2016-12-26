#include <boost/test/unit_test.hpp>

#include "common.h"
#include "attr.inl"
#include "datablock.inl"
#include "metadata.inl"

struct TestStruct {
	bool operator == (const TestStruct& ts) const { return true; }
};


BOOST_AUTO_TEST_CASE(input_attr_instantiation) {
	Metadata meta("test_type");

	{
		// in attribute instantiation -> not initialised
		InAttr<float> attr;

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
		InAttr<TestStruct> attr;

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
		OutAttr<float> attr;

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
		OutAttr<TestStruct> attr;

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
}
