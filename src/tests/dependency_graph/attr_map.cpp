#include <boost/test/unit_test.hpp>

#include <dependency_graph/attr_map.h>
#include <dependency_graph/graph.h>

using namespace dependency_graph;

namespace {

bool compare(const AttrMap& map, const std::vector<std::pair<unsigned,unsigned>>& data) {
	if(map.size() != data.size())
		return false;

	auto i1 = map.begin();
	auto i2 = data.begin();

	while(i1 != map.end()) {
		if(i1->first != i2->first || i1->second != i2->second)
			return false;
		++i1;
		++i2;
	}

	return true;
}

std::unique_ptr<MetadataHandle> meta_1;

}

////////

BOOST_AUTO_TEST_CASE(attr_map_same_map) {
	// construct the metadata 1
	{
		std::unique_ptr<Metadata> ptr(new Metadata("test_type"));

		InAttr<float> in1;
		ptr->addAttribute(in1, "input_1");

		InAttr<int> in2;
		ptr->addAttribute(in2, "input_2");

		OutAttr<float> out1;
		ptr->addAttribute(out1, "output_1");

		OutAttr<int> out2;
		ptr->addAttribute(out2, "output_2");

		meta_1 = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(ptr)));
	}

	// and make the attr map
	const AttrMap map(*meta_1, *meta_1);

	// check that its correct
	BOOST_CHECK(compare(map, {{
		{0,0}, {1,1}, {2,2}, {3,3}
	}}));
}

BOOST_AUTO_TEST_CASE(attr_map_flip_map) {
	std::unique_ptr<MetadataHandle> meta_2;

	// construct the metadata 2
	{
		std::unique_ptr<Metadata> ptr(new Metadata("test_type"));

		InAttr<int> in2;
		ptr->addAttribute(in2, "input_2");

		OutAttr<float> out1;
		ptr->addAttribute(out1, "output_1");

		InAttr<float> in1;
		ptr->addAttribute(in1, "input_1");

		OutAttr<int> out2;
		ptr->addAttribute(out2, "output_2");

		meta_2 = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(ptr)));
	}

	// and make the attr map
	const AttrMap map(*meta_2, *meta_1);

	// check that its correct
	BOOST_CHECK(compare(map, {{
		{0,1}, {1,2}, {2,0}, {3,3}
	}}));

	// and make the reversed attr map
	const AttrMap map_rev(*meta_1, *meta_2);

	// check that its correct (flipped attributes are correctly matched)
	BOOST_CHECK(compare(map_rev, {{
		{0,2}, {1,0}, {2,1}, {3,3}
	}}));
}

BOOST_AUTO_TEST_CASE(attr_map_rename_attribute) {
	std::unique_ptr<MetadataHandle> meta_2;

	// construct the metadata
	{
		std::unique_ptr<Metadata> ptr(new Metadata("test_type"));

		InAttr<float> in1;
		ptr->addAttribute(in1, "input_1");

		InAttr<int> in2;
		ptr->addAttribute(in2, "input_2_renamed");

		OutAttr<float> out1;
		ptr->addAttribute(out1, "output_1");

		OutAttr<int> out2;
		ptr->addAttribute(out2, "output_2");

		meta_2 = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(ptr)));
	}

	// and make the attr map
	const AttrMap map(*meta_1, *meta_2);

	// check that its correct (renamed attr is correctly matched)
	BOOST_CHECK(compare(map, {{
		{0,0}, {1,1}, {2,2}, {3,3}
	}}));

	// and make the reversed attr map
	const AttrMap map_rev(*meta_2, *meta_1);

	// check that its correct (renamed attr is correctly matched)
	BOOST_CHECK(compare(map_rev, {{
		{0,0}, {1,1}, {2,2}, {3,3}
	}}));
}

BOOST_AUTO_TEST_CASE(attr_map_retype_attribute) {
	std::unique_ptr<MetadataHandle> meta_2;

	// construct the metadata
	{
		std::unique_ptr<Metadata> ptr(new Metadata("test_type"));

		InAttr<float> in1;
		ptr->addAttribute(in1, "input_1");

		InAttr<unsigned> in2;
		ptr->addAttribute(in2, "input_2");

		OutAttr<float> out1;
		ptr->addAttribute(out1, "output_1");

		OutAttr<int> out2;
		ptr->addAttribute(out2, "output_2");

		meta_2 = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(ptr)));
	}

	// and make the attr map
	const AttrMap map(*meta_1, *meta_2);

	// check that its correct (retyped attr is not matched)
	BOOST_CHECK(compare(map, {{
		{0,0}, {2,2}, {3,3}
	}}));

	// and make the reversed attr map
	const AttrMap map_rev(*meta_2, *meta_1);

	// check that its correct (retyped attr is not matched)
	BOOST_CHECK(compare(map_rev, {{
		{0,0}, {2,2}, {3,3}
	}}));
}

BOOST_AUTO_TEST_CASE(attr_map_remove_attribute) {
	std::unique_ptr<MetadataHandle> meta_2;

	// construct the metadata
	{
		std::unique_ptr<Metadata> ptr(new Metadata("test_type"));

		InAttr<float> in1;
		ptr->addAttribute(in1, "input_1");

		OutAttr<float> out1;
		ptr->addAttribute(out1, "output_1");

		OutAttr<int> out2;
		ptr->addAttribute(out2, "output_2");

		meta_2 = std::unique_ptr<MetadataHandle>(new MetadataHandle(std::move(ptr)));
	}

	// and make the attr map
	const AttrMap map(*meta_1, *meta_2);

	// check that its correct (removed attr is not matched)
	BOOST_CHECK(compare(map, {{
		{0,0}, {2,1}, {3,2}
	}}));

	// and make the reversed attr map
	const AttrMap map_rev(*meta_2, *meta_1);

	// check that its correct (removed attr is not matched)
	BOOST_CHECK(compare(map_rev, {{
		{0,0}, {1,2}, {2,3}
	}}));
}
