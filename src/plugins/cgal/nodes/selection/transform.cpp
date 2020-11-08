#include <possumwood_sdk/node_implementation.h>

#include <OpenEXR/ImathEuler.h>
#include <OpenEXR/ImathVec.h>

#include "cgal.h"
#include "datatypes/selection.h"
#include "maths/io/vec3.h"
#include "transform.h"

namespace {

using possumwood::FaceSelection;
using possumwood::Meshes;

dependency_graph::InAttr<FaceSelection> a_inSelection;
dependency_graph::InAttr<Imath::Vec3<float>> a_translation, a_rotation, a_scale;
dependency_graph::InAttr<bool> a_vec3AsNormal;
dependency_graph::OutAttr<FaceSelection> a_outSelection;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Imath::Vec3<float> tr = data.get(a_translation);
	const Imath::Vec3<float> rot = data.get(a_rotation);
	const Imath::Vec3<float> sc = data.get(a_scale);

	Imath::Matrix44<float> m1, m2, m3;
	m1 = Imath::Euler<float>(Imath::Vec3<float>(rot.x * M_PI / 180.0, rot.y * M_PI / 180.0, rot.z * M_PI / 180.0))
	         .toMatrix44();
	m2.setScale(sc);
	m3.setTranslation(tr);

	Meshes tmp;
	for(auto& i : data.get(a_inSelection))
		tmp.addMesh(i.mesh());

	tmp = transform(tmp, m1 * m2 * m3, data.get(a_vec3AsNormal));

	FaceSelection result;
	for(std::size_t i = 0; i < tmp.size(); ++i) {
		FaceSelection::Item item(tmp[i]);
		const FaceSelection::Item& src = data.get(a_inSelection)[i];

		for(auto h = src.mesh().polyhedron().facets_begin(); h != src.mesh().polyhedron().facets_end(); ++h)
			if(src.contains(h))
				item.push_back(h);

		result.push_back(item);
	}

	data.set(a_outSelection, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSelection, "in_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_translation, "translation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_rotation, "rotation", Imath::Vec3<float>(0, 0, 0));
	meta.addAttribute(a_scale, "scale", Imath::Vec3<float>(1, 1, 1));
	meta.addAttribute(a_vec3AsNormal, "vec3_as_normals", true);
	meta.addAttribute(a_outSelection, "out_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inSelection, a_outSelection);
	meta.addInfluence(a_translation, a_outSelection);
	meta.addInfluence(a_rotation, a_outSelection);
	meta.addInfluence(a_scale, a_outSelection);
	meta.addInfluence(a_vec3AsNormal, a_outSelection);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/selection/transform", init);
}  // namespace
