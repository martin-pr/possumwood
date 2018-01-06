// #include <possumwood_sdk/node_implementation.h>

// #include <OpenEXR/ImathVec.h>
// #include <OpenEXR/ImathEuler.h>

// #include "maths/io/vec3.h"
// #include "datatypes/meshes.h"
// #include "cgal.h"

// namespace {

// using possumwood::Meshes;
// using possumwood::CGALPolyhedron;

// dependency_graph::InAttr<Meshes> a_inMesh;
// dependency_graph::InAttr<Imath::Vec3<float>> a_translation, a_rotation, a_scale;
// dependency_graph::OutAttr<Meshes> a_outMesh;

// dependency_graph::State compute(dependency_graph::Values& data) {
// 	const Imath::Vec3<float> tr = data.get(a_translation);
// 	const Imath::Vec3<float> rot = data.get(a_rotation);
// 	const Imath::Vec3<float> sc = data.get(a_scale);

// 	Imath::Matrix44<float> m1, m2, m3;
// 	m1 = Imath::Euler<float>(Imath::Vec3<float>(rot.x * M_PI / 180.0,
// 	                                            rot.y * M_PI / 180.0,
// 	                                            rot.z * M_PI / 180.0))
// 	         .toMatrix44();
// 	m2.setScale(sc);
// 	m3.setTranslation(tr);

// 	const Imath::Matrix44<float> matrix = m1 * m2 * m3;

// 	Meshes result;
// 	for(auto& inMesh : data.get(a_inMesh)) {
// 		std::unique_ptr<possumwood::CGALPolyhedron> newMesh(
// 		    new possumwood::CGALPolyhedron(inMesh.mesh()));

// 		for(auto it = newMesh->vertices_begin(); it != newMesh->vertices_end();
// 		    ++it) {
// 			possumwood::CGALPolyhedron::Point& pt = newMesh->point(*it);

// 			Imath::Vec3<float> p(pt[0], pt[1], pt[2]);
// 			p *= matrix;

// 			pt = possumwood::CGALKernel::Point_3(p.x, p.y, p.z);
// 		}

// 		result.addMesh(inMesh.name(), std::move(newMesh));
// 	}

// 	data.set(a_outMesh, result);

// 	return dependency_graph::State();
// }

// void init(possumwood::Metadata& meta) {
// 	meta.addAttribute(a_inMesh, "in_mesh");
// 	meta.addAttribute(a_translation, "translation",
// 	                  Imath::Vec3<float>(0, 0, 0));
// 	meta.addAttribute(a_rotation, "rotation", Imath::Vec3<float>(0, 0, 0));
// 	meta.addAttribute(a_scale, "scale", Imath::Vec3<float>(1, 1, 1));
// 	meta.addAttribute(a_outMesh, "out_mesh");

// 	meta.addInfluence(a_inMesh, a_outMesh);
// 	meta.addInfluence(a_translation, a_outMesh);
// 	meta.addInfluence(a_rotation, a_outMesh);
// 	meta.addInfluence(a_scale, a_outMesh);

// 	meta.setCompute(compute);
// }

// possumwood::NodeImplementation s_impl("cgal/transform", init);
// }
