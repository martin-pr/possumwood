#include <possumwood_sdk/node_implementation.h>

// HACK - c++11 and CGAL don't seem to go well together, unfortunately
namespace CGAL {
namespace cpp11 {
template <typename ITER>
ITER prev(ITER i) {
	return std::prev(i);
}

template <typename ITER>
ITER next(ITER i) {
	return std::next(i);
}
}
}

#include <CGAL/Surface_mesh_simplification/edge_collapse.h>

#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>

#include "datatypes/polyhedron.h"

#include "cgal.h"

namespace SMS = CGAL::Surface_mesh_simplification;

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::CGALPolyhedron>> a_inMesh;
dependency_graph::InAttr<float> a_stopCondition;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::CGALPolyhedron>> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::shared_ptr<const possumwood::CGALPolyhedron> inMesh = data.get(a_inMesh);
	const float stopCondition = data.get(a_stopCondition);

	if(inMesh) {
		std::unique_ptr<possumwood::CGALPolyhedron> mesh(new possumwood::CGALPolyhedron(*inMesh));

		SMS::edge_collapse(*mesh, SMS::Count_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
		                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
		                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));

		data.set(a_outMesh, std::shared_ptr<const possumwood::CGALPolyhedron>(mesh.release()));
	}
	else
		data.set(a_outMesh, std::shared_ptr<const possumwood::CGALPolyhedron>());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "input");
	meta.addAttribute(a_stopCondition, "stop_condition", 1000.0f);
	meta.addAttribute(a_outMesh, "output");

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_stopCondition, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/simplification/simplification", init);
}
