#include <possumwood_sdk/node_implementation.h>

#include <algorithm>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathEuler.h>

#include <CGAL/HalfedgeDS_decorator.h>

#include "maths/io/vec3.h"
#include "datatypes/meshes.h"
#include "cgal.h"

namespace {

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

dependency_graph::InAttr<Meshes> a_inMesh;
dependency_graph::InAttr<Imath::Vec3<float>> a_translation, a_rotation, a_scale;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Imath::Vec3<float> tr = data.get(a_translation);
	const Imath::Vec3<float> rot = data.get(a_rotation);
	const Imath::Vec3<float> sc = data.get(a_scale);

	Imath::Matrix44<float> m1, m2, m3;
	m1 =
	    Imath::Euler<float>(Imath::Vec3<float>(rot.x * M_PI / 180.0, rot.y * M_PI / 180.0,
	                                           rot.z * M_PI / 180.0))
	        .toMatrix44();
	m2.setScale(sc);
	m3.setTranslation(tr);

	const Imath::Matrix44<float> matrix = m1 * m2 * m3;

	// if the scale values invert the mesh, we need to flip the polygons to make it display correctly
	const bool meshNeedsFlipping = sc[0]*sc[1]*sc[2] < 0.0f;

	Meshes result = data.get(a_inMesh);
	for(auto& mesh : result) {
		for(auto it = mesh.polyhedron().vertices_begin();
		    it != mesh.polyhedron().vertices_end(); ++it) {
			possumwood::CGALPolyhedron::Point& pt = it->point();;

			Imath::Vec3<float> p(pt[0], pt[1], pt[2]);
			p *= matrix;

			pt = possumwood::CGALKernel::Point_3(p.x, p.y, p.z);
		}

		// turn the mesh inside out
		if(meshNeedsFlipping) {
			CGAL::HalfedgeDS_decorator<possumwood::CGALPolyhedron::HalfedgeDS> decorator(mesh.polyhedron().hds());
			decorator.inside_out();
		}
	}

	data.set(a_outMesh, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "in_mesh");
	meta.addAttribute(a_translation, "translation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_rotation, "rotation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_scale, "scale", Imath::Vec3<float>(1, 1, 1));
	meta.addAttribute(a_outMesh, "out_mesh");

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_translation, a_outMesh);
	meta.addInfluence(a_rotation, a_outMesh);
	meta.addInfluence(a_scale, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/transform", init);
}
