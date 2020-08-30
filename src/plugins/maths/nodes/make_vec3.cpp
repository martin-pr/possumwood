#include <OpenEXR/ImathVec.h>
#include <possumwood_sdk/node_implementation.h>

#include "io/vec3.h"

namespace {

template <typename T>
struct Vec3Node {
	struct Params {
		dependency_graph::InAttr<T> a_x, a_y, a_z;
		dependency_graph::OutAttr<Imath::Vec3<T>> a_out;
	};

	static Params& params() {
		static Params s_params;
		return s_params;
	}

	static dependency_graph::State compute(dependency_graph::Values& data) {
		const T x = data.get(params().a_x);
		const T y = data.get(params().a_y);
		const T z = data.get(params().a_z);

		data.set(params().a_out, Imath::Vec3<T>(x, y, z));

		return dependency_graph::State();
	}

	static void init(possumwood::Metadata& meta) {
		meta.addAttribute(params().a_x, "x");
		meta.addAttribute(params().a_y, "y");
		meta.addAttribute(params().a_z, "z");
		meta.addAttribute(params().a_out, "out", Imath::Vec3<T>(0, 0, 0));

		meta.addInfluence(params().a_x, params().a_out);
		meta.addInfluence(params().a_y, params().a_out);
		meta.addInfluence(params().a_z, params().a_out);

		meta.setCompute(compute);
	}
};

possumwood::NodeImplementation s_implf("maths/make_vec3", Vec3Node<float>::init);
possumwood::NodeImplementation s_implu("maths/make_vec3u", Vec3Node<unsigned>::init);
possumwood::NodeImplementation s_impli("maths/make_vec3i", Vec3Node<int>::init);

}  // namespace
