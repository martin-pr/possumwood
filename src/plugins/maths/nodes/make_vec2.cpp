#include <possumwood_sdk/node_implementation.h>

#include <OpenEXR/ImathVec.h>

#include "io/vec2.h"

namespace {

template<typename T>
struct Vec2Node {

struct Params {
	dependency_graph::InAttr<T> a_x, a_y;
	dependency_graph::OutAttr<Imath::Vec2<T>> a_out;
};

static Params& params() {
	static Params s_params;
	return s_params;
}

static dependency_graph::State compute(dependency_graph::Values& data) {
	const T x = data.get(params().a_x);
	const T y = data.get(params().a_y);

	data.set(params().a_out, Imath::Vec2<T>(x, y));

	return dependency_graph::State();
}

static void init(possumwood::Metadata& meta) {
	meta.addAttribute(params().a_x, "x");
	meta.addAttribute(params().a_y, "y");
	meta.addAttribute(params().a_out, "out", Imath::Vec2<T>(0, 0));

	meta.addInfluence(params().a_x, params().a_out);
	meta.addInfluence(params().a_y, params().a_out);

	meta.setCompute(compute);
}

};

possumwood::NodeImplementation s_implf("maths/make_vec2", Vec2Node<float>::init);
possumwood::NodeImplementation s_implu("maths/make_vec2u", Vec2Node<unsigned>::init);
possumwood::NodeImplementation s_impli("maths/make_vec2i", Vec2Node<int>::init);

}
