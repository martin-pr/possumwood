#include <possumwood_sdk/datatypes/enum.h>
#include <possumwood_sdk/node_implementation.h>

#include <CGAL/AABB_face_graph_triangle_primitive.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Side_of_triangle_mesh.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/algorithm.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>

#include "datatypes/meshes.h"
#include "datatypes/selection.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::FaceSelection;
using possumwood::Meshes;

typedef possumwood::CGALPolyhedron Polyhedron;
typedef possumwood::CGALKernel Kernel;

typedef CGAL::AABB_face_graph_triangle_primitive<Polyhedron> Primitive;
typedef CGAL::AABB_traits<Kernel, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;
typedef CGAL::Side_of_triangle_mesh<Polyhedron, Kernel> Point_inside;

dependency_graph::InAttr<FaceSelection> a_inSelection;
dependency_graph::InAttr<Meshes> a_mesh;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<FaceSelection> a_outSelection;

enum Mode { kAdd, kRemove, kSelectedOnly };

std::vector<std::pair<std::string, Mode>> s_enum{{"Add to selection", kAdd},
                                                 {"Remove from selection", kRemove},
                                                 {"Replace selection", kSelectedOnly}};

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::ScopedOutputRedirect redirect;

	// first build a tree that contains all the meshes in the input mesh
	Tree tree;
	for(auto mesh : data.get(a_mesh)) {
		if(!CGAL::is_triangle_mesh(mesh.polyhedron()))
			throw std::runtime_error("Only triangulated meshes can be used as an input to this node.");

		tree.insert(CGAL::faces(mesh.polyhedron()).first, CGAL::faces(mesh.polyhedron()).second, mesh.polyhedron());
	}
	tree.accelerate_distance_queries();

	Point_inside inside_tester(tree);

	FaceSelection selection;

	const Mode mode = (Mode)data.get(a_mode).intValue();

	std::size_t i = 0;
	for(auto current : data.get(a_inSelection)) {
		FaceSelection::Item item(current.mesh());
		if(mode == kAdd || mode == kRemove)
			item = current;

		for(auto h = current.mesh().polyhedron().facets_begin(); h != current.mesh().polyhedron().facets_end(); ++h) {
			bool inside = true;
			auto it = h->facet_begin();
			for(std::size_t i = 0; i < h->facet_degree(); ++i, ++it)
				if(inside_tester(it->vertex()->point()) != CGAL::ON_BOUNDED_SIDE)
					inside = false;

			switch(mode) {
				case kSelectedOnly:
				case kAdd:
					if(inside)
						item.push_back(h);
					break;

				case kRemove:
					if(inside)
						item.remove(h);
					break;
			}
		}

		selection.push_back(item);

		++i;
	}

	data.set(a_outSelection, selection);

	return redirect.state();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSelection, "in_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mesh, "mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_enum.begin(), s_enum.end()));
	meta.addAttribute(a_outSelection, "out_selection", possumwood::FaceSelection(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inSelection, a_outSelection);
	meta.addInfluence(a_mesh, a_outSelection);
	meta.addInfluence(a_mode, a_outSelection);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/selection/point_inside_poly", init);
}  // namespace
