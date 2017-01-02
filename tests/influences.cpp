#include <boost/test/unit_test.hpp>

#include "attr.inl"
#include "metadata.inl"
#include "datablock.inl"
#include "common.h"

typedef std::vector<std::reference_wrapper<const Attr>> Influences;

bool operator ==(const Influences i1, const Influences i2) {
	auto it1 = i1.begin();
	auto it2 = i2.begin();
	while((it1 != i1.end()) && (it2 != i2.end()))
		if(it1->get() != it2->get())
			return false;
		else {
			++it1;
			++it2;
		}

	return it1 == i1.end() && it2 == i2.end();
}

namespace std {
	std::ostream& operator << (std::ostream& out, const Influences& attr) {
		for(auto& a : attr)
			out << a.get().name();
		return out;
	}
}

BOOST_AUTO_TEST_CASE(metadata_influences) {
	// construct the metadata
	Metadata meta("test_type");

	InAttr<float> in1;
	BOOST_REQUIRE_NO_THROW(meta.addAttribute(in1, "input_1"));

	InAttr<TestStruct> in2;
	BOOST_REQUIRE_NO_THROW(meta.addAttribute(in2, "input_2"));

	InAttr<std::string> in3;
	BOOST_REQUIRE_NO_THROW(meta.addAttribute(in3, "input_3"));

	OutAttr<float> out1;
	BOOST_REQUIRE_NO_THROW(meta.addAttribute(out1, "output_1"));

	OutAttr<TestStruct> out2;
	BOOST_REQUIRE_NO_THROW(meta.addAttribute(out2, "output_2"));

	OutAttr<std::string> out3;
	BOOST_REQUIRE_NO_THROW(meta.addAttribute(out3, "output_3"));

	// add some influences
	BOOST_REQUIRE_NO_THROW(meta.addInfluence(in1, out1));

	BOOST_REQUIRE_NO_THROW(meta.addInfluence(in1, out2));
	BOOST_REQUIRE_NO_THROW(meta.addInfluence(in2, out2));

	// and test the result
	BOOST_CHECK_EQUAL(meta.influences(in3), Influences{});
	BOOST_CHECK_EQUAL(meta.influencedBy(out3), Influences{});

	BOOST_CHECK_EQUAL(meta.influences(in2), Influences{out2});
	BOOST_CHECK_EQUAL(meta.influencedBy(out1), Influences{in1});

	const Influences ii1{out1, out2};
	BOOST_CHECK_EQUAL(meta.influences(in1), ii1);
	const Influences ii2{in1, in2};
	BOOST_CHECK_EQUAL(meta.influencedBy(out2), ii2);
}
