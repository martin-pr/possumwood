#include <possumwood_sdk/node_implementation.h>

#include <OpenEXR/ImathEuler.h>
#include <OpenEXR/ImathVec.h>

#include "cgal.h"
#include "datatypes/meshes.h"
#include "maths/io/vec3.h"

namespace {

using possumwood::CGALPolyhedron;

template <typename OP>
struct BinaryOp {
	struct Params {
		dependency_graph::InAttr<possumwood::CGALNefPolyhedron> a_in1, a_in2;
		dependency_graph::OutAttr<possumwood::CGALNefPolyhedron> a_out;
	};

	static dependency_graph::State compute(dependency_graph::Values& data, Params& params) {
		std::unique_ptr<possumwood::CGALNef> nef(new possumwood::CGALNef(*data.get(params.a_in1)));
		OP::exec(*nef, *data.get(params.a_in2));

		data.set(params.a_out, possumwood::CGALNefPolyhedron(std::move(nef)));

		return dependency_graph::State();
	}

	static void init(possumwood::Metadata& meta) {
		static Params s_params;

		meta.addAttribute(s_params.a_in1, "nef_1", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);
		meta.addAttribute(s_params.a_in2, "nef_2", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);
		meta.addAttribute(s_params.a_out, "nef", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);

		meta.addInfluence(s_params.a_in1, s_params.a_out);
		meta.addInfluence(s_params.a_in2, s_params.a_out);

		meta.setCompute([&](dependency_graph::Values& data) { return compute(data, s_params); });
	}
};

struct UnionOp {
	static void exec(possumwood::CGALNef& arg1, const possumwood::CGALNef& arg2) {
		arg1 += arg2;
	}
};

struct DifferenceOp {
	static void exec(possumwood::CGALNef& arg1, const possumwood::CGALNef& arg2) {
		arg1 -= arg2;
	}
};

struct IntersectionOp {
	static void exec(possumwood::CGALNef& arg1, const possumwood::CGALNef& arg2) {
		arg1 *= arg2;
	}
};

struct SymDiffOp {
	static void exec(possumwood::CGALNef& arg1, const possumwood::CGALNef& arg2) {
		arg1 ^= arg2;
	}
};

possumwood::NodeImplementation s_implUnion("cgal/csg/union", BinaryOp<UnionOp>::init);
possumwood::NodeImplementation s_implDifference("cgal/csg/difference", BinaryOp<DifferenceOp>::init);
possumwood::NodeImplementation s_implIntersection("cgal/csg/intersection", BinaryOp<IntersectionOp>::init);
possumwood::NodeImplementation s_implSymDiff("cgal/csg/symmetric_diference", BinaryOp<SymDiffOp>::init);

}  // namespace
