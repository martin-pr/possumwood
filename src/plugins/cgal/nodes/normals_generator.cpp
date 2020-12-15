#include <CGAL/Polygon_mesh_processing/compute_normal.h>

#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include "datatypes/meshes.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<Meshes> a_inMeshes;
dependency_graph::InAttr<std::string> a_attr;
dependency_graph::OutAttr<Meshes> a_outMesh;

float vec3_len(const possumwood::CGALKernel::Vector_3& v) {
	return sqrt(v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
}

possumwood::CGALKernel::Point_3 operator*(const possumwood::CGALKernel::Point_3& p, float s) {
	return possumwood::CGALKernel::Point_3(p.x() * s, p.y() * s, p.z() * s);
}

possumwood::CGALKernel::Point_3 operator+(const possumwood::CGALKernel::Point_3& p1,
                                          const possumwood::CGALKernel::Point_3& p2) {
	return possumwood::CGALKernel::Point_3(p1.x() + p2.x(), p1.y() + p2.y(), p1.z() + p2.z());
}

struct FakeKernel {
	typedef float FT;

	typedef possumwood::CGALKernel::Point_3 Point_3;

	struct Vector_3 {
		Vector_3() : data{{0, 0, 0}} {
		}

		Vector_3(float x, float y, float z) : data{{x, y, z}} {
		}

		Vector_3(const std::array<float, 3>& arr) : data(arr) {
		}

		Vector_3(const CGAL::Null_vector) : data{{0, 0, 0}} {
		}

		float& operator[](std::size_t index) {
			return data[index];
		}

		const float& operator[](std::size_t index) const {
			return data[index];
		}

		operator const std::array<float, 3> &() const {
			return data;
		}

		bool operator==(const Vector_3& v) const {
			return v.data == data;
		}

		bool operator!=(const Vector_3& v) const {
			return v.data != data;
		}

		bool operator==(const CGAL::Null_vector&) const {
			return data[0] == 0.0f && data[1] == 0.0f && data[2] == 0.0f;
		}

		bool operator!=(const CGAL::Null_vector) const {
			return data[0] != 0.0f || data[1] != 0.0f || data[2] != 0.0f;
		}

		std::array<float, 3> data;
	};

	struct Equal_3 {
		bool operator()(const Vector_3& p, CGAL::Null_vector) const {
			return p[0] == 0 && p[1] == 0 && p[2] == 0;
		}
	};

	struct Construct_vector_3 {
		Vector_3 operator()(CGAL::Null_vector) const {
			return Vector_3(CGAL::Null_vector());
		}

		Vector_3 operator()(const Vector_3& val) const {
			return val;
		}

		Vector_3 operator()(const Point_3& p1, const Point_3& p2) const {
			return Vector_3(p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2]);
		}
	};

	Construct_vector_3 construct_vector_3_object() const {
		return Construct_vector_3();
	}

	struct Construct_sum_of_vectors_3 {
		Vector_3 operator()(const Vector_3& p1, const Vector_3& p2) {
			return Vector_3(p1[0] + p2[0], p1[1] + p2[1], p1[2] + p2[2]);
		}
	};

	Construct_sum_of_vectors_3 construct_sum_of_vectors_3_object() const {
		return Construct_sum_of_vectors_3();
	}

	auto construct_divided_vector_3_object() const {
		return [](const Vector_3& p1, float v) { return std::array<float, 3>{{p1[0] / v, p1[1] / v, p1[2] / v}}; };
	}

	struct Construct_scaled_vector_3 {
		Vector_3 operator()(const Vector_3& v, float s) const {
			return Vector_3(v[0] * s, v[1] * s, v[2] * s);
		}
	};

	Construct_scaled_vector_3 construct_scaled_vector_3_object() const {
		return Construct_scaled_vector_3();
	}

	struct Compute_squared_length_3 {
		float operator()(const Vector_3& v1) const {
			return v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2];
		}
	};

	Compute_squared_length_3 compute_squared_length_3_object() const {
		return Compute_squared_length_3();
	}

	struct Compute_scalar_product_3 {
		float operator()(const Vector_3& p1, const Vector_3& p2) const {
			return p1[0]*p2[0] + p1[1]*p2[1] + p1[2]*p2[2];
		}
	};

	Compute_scalar_product_3 compute_scalar_product_3_object() const {
		return Compute_scalar_product_3();
	}

	    auto
	    construct_cross_product_vector_3_object() const {
		return [](const Vector_3& p1, const Vector_3& p2) {
			return std::array<float, 3>{
			    {p1[1] * p2[2] - p1[2] * p2[1], p1[2] * p2[0] - p1[0] * p2[2], p1[0] * p2[1] - p1[1] * p2[0]}};
		};
	}

	auto equal_3_object() const {
		return
		    [](const Vector_3& p1, const Vector_3& p2) { return p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2]; };
	}

	auto construct_opposite_vector_3_object() const {
		return [](const Vector_3& v) { return Vector_3(-v[0], -v[1], -v[2]); };
	}

	struct Construct_circumcenter_3 {
		// implements https://en.wikipedia.org/wiki/Circumscribed_circle#Barycentric_coordinates
		Point_3 operator()(const Point_3& A, const Point_3& B, const Point_3& C) const {
			const float a = vec3_len(B - C);
			const float b = vec3_len(A - C);
			const float c = vec3_len(A - B);

			float bary_a = a * a * (b * b + c * c - a * a);
			float bary_b = b * b * (c * c + a * a - b * b);
			float bary_c = c * c * (a * a + b * b - c * c);
			const float bary_sum = bary_a + bary_b + bary_c;
			bary_a /= bary_sum;
			bary_b /= bary_sum;
			bary_c /= bary_sum;

			return A * bary_a + B * bary_b + C * bary_c;
		}
	};

	Construct_circumcenter_3 construct_circumcenter_3_object() const {
		return Construct_circumcenter_3();
	}

	auto collinear_3_object() const {
		possumwood::CGALKernel tmp;
		return tmp.collinear_3_object();
	}
};

possumwood::CGALKernel::Point_3 operator+(const CGAL::Origin&,
                                          const FakeKernel::Vector_3& p) {
	return possumwood::CGALKernel::Point_3(p[0], p[1], p[2]);
}

template <typename PROPERTY, typename ITERATOR>
void put(std::reference_wrapper<PROPERTY>& prop, const ITERATOR& target, const FakeKernel::Vector_3& norm) {
	put(prop.get(), target->property_key(), norm);
}

template<typename PROP_T, typename VAL_T>
void put(possumwood::Property<PROP_T>& prop, const possumwood::PropertyKey& key, const VAL_T& value) {
	prop.set(key, value);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	const possumwood::Enum mode = data.get(a_mode);
	const std::string attr_name = data.get(a_attr);

	Meshes result = data.get(a_inMeshes);
	for(auto& mesh : result) {
		// request for vertex normals
		if(mode.value() == "Per-vertex normals") {
			auto& editableMesh = mesh.edit();

			// remove face normals, if they exist
			if(editableMesh.faceProperties().hasProperty(attr_name))
				editableMesh.faceProperties().removeProperty(attr_name);

			auto& normals = editableMesh.vertexProperties().addProperty(attr_name, std::array<float, 3>{{0, 0, 0}});

			CGAL::Polygon_mesh_processing::compute_vertex_normals(
			    editableMesh.polyhedron(), std::ref(normals),
			    CGAL::Polygon_mesh_processing::parameters::geom_traits(FakeKernel()));
		}

		// request for face normals
		else if(mode.value() == "Per-face normals") {
			auto& editableMesh = mesh.edit();

			// remove vertex normals, if they exist
			if(editableMesh.vertexProperties().hasProperty(attr_name))
				editableMesh.vertexProperties().removeProperty(attr_name);

			auto& normals = editableMesh.faceProperties().addProperty(attr_name, std::array<float, 3>{{0, 0, 0}});

			CGAL::Polygon_mesh_processing::compute_face_normals(
			    editableMesh.polyhedron(), std::ref(normals),
			    CGAL::Polygon_mesh_processing::parameters::geom_traits(FakeKernel()));
		}
	}

	data.set(a_outMesh, result);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"Per-face normals", "Per-vertex normals"}));
	meta.addAttribute(a_inMeshes, "input", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_attr, "attr_name", std::string("N"));
	meta.addAttribute(a_outMesh, "output", possumwood::Meshes(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_mode, a_outMesh);
	meta.addInfluence(a_inMeshes, a_outMesh);
	meta.addInfluence(a_attr, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/normals_generator", init);

}  // namespace
