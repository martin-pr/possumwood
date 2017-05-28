#include <boost/test/unit_test.hpp>

#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/datablock.inl>
#include "common.h"

using namespace dependency_graph;

typedef std::vector<std::reference_wrapper<const Attr>> Influences;

namespace std {

bool operator ==(const reference_wrapper<const Attr>& i1, const reference_wrapper<const Attr>& i2) {
	return i1.get() == i2.get();
}

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
