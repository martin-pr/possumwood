#include <possumwood_sdk/node_implementation.h>

#include <OpenEXR/ImathVec.h>

#include "io/vec3.h"

namespace {

template<typename T>
struct Vec3Node {

struct Params {
	dependency_graph::InAttr<Imath::Vec3<T>> a_in;
	dependency_graph::OutAttr<T> a_x, a_y, a_z;
};

static Params& params() {
	static Params s_params;
	return s_params;
}

static dependency_graph::State compute(dependency_graph::Values& data) {
	const Imath::Vec3<T>& vec = data.get(params().a_in);

	data.set(params().a_x, vec[0]);
	data.set(params().a_y, vec[1]);
	data.set(params().a_z, vec[2]);

	return dependency_graph::State();
}

static void init(possumwood::Metadata& meta) {
	meta.addAttribute(params().a_in, "vec", Imath::Vec3<T>(0, 0, 0));
	meta.addAttribute(params().a_x, "x");
	meta.addAttribute(params().a_y, "y");
	meta.addAttribute(params().a_z, "z");

	meta.addInfluence(params().a_in, params().a_x);
	meta.addInfluence(params().a_in, params().a_y);
	meta.addInfluence(params().a_in, params().a_z);

	meta.setCompute(compute);
}

};

possumwood::NodeImplementation s_implf("maths/split_vec3", Vec3Node<float>::init);
possumwood::NodeImplementation s_implu("maths/split_vec3u", Vec3Node<unsigned>::init);
possumwood::NodeImplementation s_impli("maths/split_vec3i", Vec3Node<int>::init);

}
