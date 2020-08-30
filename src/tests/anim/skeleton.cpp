#include <queue>
#include <sstream>

#define private public
#include "anim/datatypes/skeleton.h"
#undef private

#include <boost/test/unit_test.hpp>

using std::cout;
using std::endl;

using anim::Transform;

struct SkeletonTest {
	struct Item {
		std::string name;
		int parent;
	};

	std::vector<Item> flatten() const {
		std::vector<Item> result;

		std::queue<std::pair<SkeletonTest, int>> q;
		q.push(std::make_pair(*this, -1));

		while(!q.empty()) {
			const auto current = q.front();
			q.pop();

			result.push_back(Item{current.first.name, current.second});

			for(auto& c : current.first.children)
				q.push(std::make_pair(c, result.size() - 1));
		}

		return result;
	}

	SkeletonTest& operator[](std::size_t index) {
		assert(index < size());

		std::queue<SkeletonTest*> queue;
		queue.push(this);

		while(queue.size() <= index) {
			const auto current = queue.front();
			queue.pop();
			--index;

			for(auto& c : current->children)
				queue.push(&c);
		}

		while(index > 0) {
			queue.pop();
			--index;
		}

		return *queue.front();
	}

	std::string name;
	std::vector<SkeletonTest> children;

	// recursively computes size of the tree
	std::size_t size() const {
		std::size_t result = 1;
		for(auto& a : children)
			result += a.size();

		return result;
	}
};

///////////////////////

// the weak test just tests if the parent's ID is lower than children's, and that the children IDs don't overlap
template <typename SKELETON>
void weakTest(SKELETON& h) {
	unsigned counter = 0, childId = 1;
	for(auto& bone : h) {
		BOOST_CHECK(bone.children().m_begin > counter);

		BOOST_CHECK(bone.children().m_begin <= bone.children().m_end);
		BOOST_CHECK_EQUAL(bone.children().m_begin, childId);

		childId = bone.children().m_end;

		BOOST_CHECK((!bone.hasParent() && counter == 0) || (h.indexOf(bone.parent()) < counter));

		++counter;
	}
}

// strong test tests for exact equivalence
template <typename SKELETON>
void strongTest(SKELETON& h, const SkeletonTest& test) {
	BOOST_CHECK(not h.empty());
	BOOST_CHECK_EQUAL(h.size(), test.size());

	const auto flat = test.flatten();

	// for(auto& a : flat) {
	// 	cout << a.name << "  " << a.parent << endl;
	// }

	// cout << endl;

	BOOST_CHECK_EQUAL(h.size(), flat.size());
	for(std::size_t a = 0; a < h.size(); ++a) {
		BOOST_CHECK_EQUAL(h[a].name(), flat[a].name);
		BOOST_CHECK_EQUAL(h[a].index(), a);
		BOOST_CHECK((!h[a].hasParent() && flat[a].parent == -1) ||
		            (h.indexOf(h[a].parent()) == (std::size_t)flat[a].parent));

		for(const auto& c : h[a].children()) {
			BOOST_CHECK_EQUAL(c.m_skeleton, &h);

			const std::size_t pi = h.indexOf(c.parent());
			const std::size_t ci = h.indexOf(c);
			BOOST_CHECK_EQUAL(pi, (unsigned)flat[ci].parent);
		}
	}
}

void doTest(const anim::Skeleton& h, const SkeletonTest& test) {
	// cout << "Skeleton:" << endl;
	// for(unsigned ji=0; ji<h.size(); ++ji) {
	// 	const anim::Skeleton::Joint& j = h[ji];
	// 	cout << ji << ": " << j.name() << " (parent = " << j.m_parent << ", children=" << j.children().m_begin << "/" <<
	// j.children().m_end << ")  -  ";

	// 	for(unsigned ci=0; ci<j.children().size(); ++ci) {
	// 		const anim::Skeleton::Joint& c = *(j.children().begin()+ci);
	// 		cout << h.indexOf(c) << "/" << c.name() << "   ";
	// 	}
	// 	cout << endl;
	// }

	const auto flat = test.flatten();

	// cout << endl << "TEST:" << endl;
	// for(unsigned a=0;a<flat.size();++a) {
	// 	cout << a << ": " << flat[a].name << " (parent = " << flat[a].parent << ")  -  ";
	// 	for(unsigned b=0;b<flat.size();++b)
	// 		if(flat[b].parent == (int)a)
	// 			cout << b << "/" << flat[b].name << "   ";
	// 	cout << endl;
	// }

	// cout << "---" << endl;

	// const test
	weakTest<const anim::Skeleton>(h);
	strongTest<const anim::Skeleton>(h, test);

	// non-const test
	anim::Skeleton h2 = h;
	weakTest<anim::Skeleton>(h2);
	strongTest<anim::Skeleton>(h2, test);
}

BOOST_AUTO_TEST_CASE(empty_properties) {
	anim::Skeleton h;
	BOOST_CHECK(h.empty());
	BOOST_CHECK_EQUAL(h.size(), 0u);

	anim::Skeleton h2(h);
	BOOST_CHECK(h2.empty());
	BOOST_CHECK_EQUAL(h2.size(), 0u);
}

BOOST_AUTO_TEST_CASE(simple_chain_basic) {
	anim::Skeleton h;

	BOOST_CHECK(h.empty());
	BOOST_CHECK_EQUAL(h.size(), 0u);

	// making simple straight chain
	h.addRoot("first", Transform());
	BOOST_CHECK(not h.empty());
	BOOST_CHECK_EQUAL(h.size(), 1u);
	BOOST_CHECK_EQUAL(h[0].name(), "first");
	BOOST_CHECK_EQUAL(h[0].children().size(), 0u);
	BOOST_CHECK_EQUAL(h[0].index(), 0u);
	BOOST_CHECK(not h[0].hasParent());
	BOOST_CHECK(h[0].children().empty());

	h.addRoot("second", Transform());
	BOOST_CHECK(not h.empty());
	BOOST_CHECK_EQUAL(h.size(), 2u);
	BOOST_CHECK_EQUAL(h[0].name(), "second");
	BOOST_CHECK_EQUAL(h[0].index(), 0u);
	BOOST_CHECK_EQUAL(h[0].children().size(), 1u);
	BOOST_CHECK(not h[0].hasParent());
	BOOST_CHECK(not h[0].children().empty());
	BOOST_CHECK_EQUAL(h[1].name(), "first");
	BOOST_CHECK_EQUAL(h[1].index(), 1u);
	BOOST_CHECK(h[1].hasParent());
	BOOST_CHECK_EQUAL(h[1].parent().index(), 0u);
	BOOST_CHECK_EQUAL(h[1].children().size(), 0u);
	BOOST_CHECK(h[1].children().empty());

	h.addRoot("third", Transform());
	BOOST_CHECK(not h.empty());
	BOOST_CHECK_EQUAL(h.size(), 3u);
	BOOST_CHECK_EQUAL(h[0].name(), "third");
	BOOST_CHECK_EQUAL(h[0].index(), 0u);
	BOOST_CHECK(not h[0].hasParent());
	BOOST_CHECK_EQUAL(h[0].children().size(), 1u);
	BOOST_CHECK(not h[0].children().empty());
	BOOST_CHECK_EQUAL(h[1].name(), "second");
	BOOST_CHECK_EQUAL(h[1].index(), 1u);
	BOOST_CHECK(h[1].hasParent());
	BOOST_CHECK_EQUAL(h[1].parent().index(), 0u);
	BOOST_CHECK_EQUAL(h[1].children().size(), 1u);
	BOOST_CHECK(not h[1].children().empty());
	BOOST_CHECK_EQUAL(h[2].name(), "first");
	BOOST_CHECK_EQUAL(h[2].index(), 2u);
	BOOST_CHECK(h[2].hasParent());
	BOOST_CHECK_EQUAL(h[2].parent().index(), 1u);
	BOOST_CHECK_EQUAL(h[2].children().size(), 0u);
	BOOST_CHECK(h[2].children().empty());
}

BOOST_AUTO_TEST_CASE(simple_chain_test) {
	anim::Skeleton h;

	h.addRoot("first", Transform());
	doTest(h, SkeletonTest{"first", {}});

	h.addRoot("second", Transform());
	doTest(h, SkeletonTest{"second", {SkeletonTest{"first", {}}}});

	h.addRoot("third", Transform());
	doTest(h, SkeletonTest{"third", {SkeletonTest{"second", {SkeletonTest{"first", {}}}}}});
}

BOOST_AUTO_TEST_CASE(adding_children) {
	anim::Skeleton h;

	h.addRoot("root", Transform());
	doTest(h, SkeletonTest{"root", {}});

	h.addChild(h[0], Transform(), "first");
	doTest(h, SkeletonTest{"root", {SkeletonTest{"first", {}}}});

	h.addChild(h[0], Transform(), "second");
	doTest(h, SkeletonTest{"root", {SkeletonTest{"first", {}}, SkeletonTest{"second", {}}}});

	h.addChild(h[1], Transform(), "first_a");
	doTest(h, SkeletonTest{"root", {SkeletonTest{"first", {SkeletonTest{"first_a", {}}}}, SkeletonTest{"second", {}}}});

	h.addChild(h[1], Transform(), "first_b");
	doTest(h, SkeletonTest{"root",
	                       {SkeletonTest{"first",
	                                     {
	                                         SkeletonTest{"first_a", {}},
	                                         SkeletonTest{"first_b", {}},
	                                     }},
	                        SkeletonTest{"second", {}}}});

	h.addChild(h[3], Transform(), "first_aa");
	doTest(h, SkeletonTest{"root",
	                       {SkeletonTest{"first",
	                                     {
	                                         SkeletonTest{"first_a", {SkeletonTest{"first_aa", {}}}},
	                                         SkeletonTest{"first_b", {}},
	                                     }},
	                        SkeletonTest{"second", {}}}});

	h.addChild(h[1], Transform(), "first_c");
	doTest(h, SkeletonTest{"root",
	                       {SkeletonTest{"first",
	                                     {
	                                         SkeletonTest{"first_a", {SkeletonTest{"first_aa", {}}}},
	                                         SkeletonTest{"first_b", {}},
	                                         SkeletonTest{"first_c", {}},
	                                     }},
	                        SkeletonTest{"second", {}}}});

	h.addChild(h[2], Transform(), "second_a");
	doTest(h, SkeletonTest{"root",
	                       {SkeletonTest{"first",
	                                     {
	                                         SkeletonTest{"first_a", {SkeletonTest{"first_aa", {}}}},
	                                         SkeletonTest{"first_b", {}},
	                                         SkeletonTest{"first_c", {}},
	                                     }},
	                        SkeletonTest{"second", {SkeletonTest{"second_a", {}}}}}});
}

// this test is such a bad idea... but, if it helps to test the thing properly, lets do it
BOOST_AUTO_TEST_CASE(randomized_children) {
	// 100 tests
	for(unsigned a = 0; a < 100; ++a) {
		// start with a single joint
		SkeletonTest test{"root", {}};

		anim::Skeleton h;
		h.addRoot("root", Transform());

		// max 30 joints
		unsigned totalCount = rand() % 80;
		for(unsigned b = 0; b < totalCount; ++b) {
			std::stringstream name;
			name << "joint_" << b;

			std::size_t index = rand() % test.size();

			test[index].children.push_back(SkeletonTest{name.str(), {}});
			h.addChild(h[index], Transform(), name.str());
		}

		// and do the tests
		doTest(h, test);
	}
}
