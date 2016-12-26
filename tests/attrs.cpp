#include <boost/test/unit_test.hpp>

#include "common.h"
#include "attr.inl"
#include "datablock.inl"

struct TestStruct {
	bool operator == (const TestStruct& ts) const { return true; }
};


BOOST_AUTO_TEST_CASE(input_attr_instantiation) {
	// in attribute instantiation -> not initialised
	{
		InAttr<float> attr;

		BOOST_CHECK(not attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "");
		BOOST_CHECK_EQUAL(attr.category(), Attr::kInput);
		BOOST_CHECK_EQUAL(attr.type(), typeid(float));


		InAttr<TestStruct> tattr;

		BOOST_CHECK(not tattr.isValid());
		BOOST_CHECK_EQUAL(tattr.name(), "");
		BOOST_CHECK_EQUAL(tattr.category(), Attr::kInput);
		BOOST_CHECK_EQUAL(tattr.type(), typeid(TestStruct));
	}

	// in attribute instantiation -> not initialised
	{
		OutAttr<float> attr;

		BOOST_CHECK(not attr.isValid());
		BOOST_CHECK_EQUAL(attr.name(), "");
		BOOST_CHECK_EQUAL(attr.category(), Attr::kOutput);
		BOOST_CHECK_EQUAL(attr.type(), typeid(float));


		OutAttr<TestStruct> tattr;

		BOOST_CHECK(not tattr.isValid());
		BOOST_CHECK_EQUAL(tattr.name(), "");
		BOOST_CHECK_EQUAL(tattr.category(), Attr::kOutput);
		BOOST_CHECK_EQUAL(tattr.type(), typeid(TestStruct));
	}
}
