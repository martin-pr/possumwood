#include "common.h"

#include <boost/test/unit_test.hpp>

#include <dependency_graph/attr.inl>
#include <dependency_graph/values.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/node.inl>

using namespace dependency_graph;

namespace std {

ostream& operator << (ostream& out, const type_info& t) {
	out << t.name();

	return out;
}

}

/////////////

namespace {
	static unsigned s_counter = 0;
}

TestStruct::TestStruct() : id(s_counter++) {
}

bool TestStruct::operator == (const TestStruct& ts) const {
	return id == ts.id;
}

bool TestStruct::operator != (const TestStruct& ts) const {
	return id != ts.id;
}

std::ostream& operator << (std::ostream& out, const TestStruct& t) {
	out << t.id;

	return out;
}

/////////////

const Metadata& additionNode() {
	static Metadata s_meta("addition");

	if(!s_meta.isValid()) {
		// create attributes
		static InAttr<float> additionInput1, additionInput2;
		static OutAttr<float> additionOutput;
		BOOST_CHECK(not additionInput1.isValid());
		BOOST_CHECK(not additionInput2.isValid());
		BOOST_CHECK(not additionOutput.isValid());

		// add attributes to the Metadata instance
		s_meta.addAttribute(additionInput1, "input_1");
		s_meta.addAttribute(additionInput2, "input_2");
		s_meta.addAttribute(additionOutput, "output");

		BOOST_REQUIRE_EQUAL(s_meta.attributeCount(), 3u);
		BOOST_CHECK_EQUAL(&s_meta.attr(0), &additionInput1);
		BOOST_CHECK_EQUAL(&s_meta.attr(1), &additionInput2);
		BOOST_CHECK_EQUAL(&s_meta.attr(2), &additionOutput);

		// setup influences
		BOOST_CHECK_NO_THROW(s_meta.addInfluence(additionInput1, additionOutput));
		BOOST_CHECK_NO_THROW(s_meta.addInfluence(additionInput2, additionOutput));

		std::vector<std::reference_wrapper<const Attr>> influences;

		BOOST_CHECK_NO_THROW(influences = s_meta.influences(additionInput1));
		BOOST_REQUIRE_EQUAL(influences.size(), 1u);
		BOOST_CHECK_EQUAL(&(influences.begin()->get()), &additionOutput);

		BOOST_CHECK_NO_THROW(influences = s_meta.influencedBy(additionOutput));
		BOOST_REQUIRE_EQUAL(influences.size(), 2u);
		BOOST_CHECK_EQUAL(&(influences[0].get()), &additionInput1);
		BOOST_CHECK_EQUAL(&(influences[1].get()), &additionInput2);

		std::function<State(Values&)> additionCompute = [&](Values& data) {
			const float a = data.get(additionInput1);
			const float b = data.get(additionInput2);

			data.set(additionOutput, a + b);

			return State();
		};
		s_meta.setCompute(additionCompute);
	}

	return s_meta;
}

const Metadata& multiplicationNode() {
	static Metadata s_meta("multiplication");

	if(!s_meta.isValid()) {
		static InAttr<float> multiplicationInput1, multiplicationInput2;
		static OutAttr<float> multiplicationOutput;
		std::function<State(Values&)> multiplicationCompute = [&](Values & data) {
			const float a = data.get(multiplicationInput1);
			const float b = data.get(multiplicationInput2);

			data.set(multiplicationOutput, a * b);

			return State();
		};

		s_meta.addAttribute(multiplicationInput1, "input_1");
		s_meta.addAttribute(multiplicationInput2, "input_2");
		s_meta.addAttribute(multiplicationOutput, "output");

		s_meta.addInfluence(multiplicationInput1, multiplicationOutput);
		s_meta.addInfluence(multiplicationInput2, multiplicationOutput);

		s_meta.setCompute(multiplicationCompute);
	}

	return s_meta;
}
