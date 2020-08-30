#include <OpenEXR/ImathVec.h>
#include <possumwood_sdk/node_implementation.h>

#include "io/vec2.h"

namespace {

template <typename T>
struct Vec2Node {
	struct Params {
		dependency_graph::InAttr<Imath::Vec2<T>> a_in;
		dependency_graph::OutAttr<T> a_x, a_y;
	};

	static Params& params() {
		static Params s_params;
		return s_params;
	}

	static dependency_graph::State compute(dependency_graph::Values& data) {
		const Imath::Vec2<T>& vec = data.get(params().a_in);

		data.set(params().a_x, vec[0]);
		data.set(params().a_y, vec[1]);

		return dependency_graph::State();
	}

	static void init(possumwood::Metadata& meta) {
		meta.addAttribute(params().a_in, "vec", Imath::Vec2<T>(0, 0));
		meta.addAttribute(params().a_x, "x");
		meta.addAttribute(params().a_y, "y");

		meta.addInfluence(params().a_in, params().a_x);
		meta.addInfluence(params().a_in, params().a_y);

		meta.setCompute(compute);
	}
};

possumwood::NodeImplementation s_implf("maths/split_vec2", Vec2Node<float>::init);
possumwood::NodeImplementation s_implu("maths/split_vec2u", Vec2Node<unsigned>::init);
possumwood::NodeImplementation s_impli("maths/split_vec2i", Vec2Node<int>::init);

}  // namespace
