#include <possumwood_sdk/node_implementation.h>

#include <CGAL/squared_distance_3.h>

#include <CGAL/Surface_mesh_simplification/edge_collapse.h>

#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_stop_predicate.h>

#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "datatypes/polyhedron.h"

#include "cgal.h"

namespace SMS = CGAL::Surface_mesh_simplification;
using namespace std::placeholders;

namespace {

dependency_graph::InAttr<std::shared_ptr<const possumwood::CGALPolyhedron>> a_inMesh;
dependency_graph::InAttr<float> a_stopParam;
dependency_graph::InAttr<possumwood::Enum> a_stopCondition;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::CGALPolyhedron>> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	std::shared_ptr<const possumwood::CGALPolyhedron> inMesh = data.get(a_inMesh);
	const float stopCondition = data.get(a_stopParam);
	const unsigned algorithmId = data.get(a_stopCondition).intValue();

	if(inMesh) {
		std::unique_ptr<possumwood::CGALPolyhedron> mesh(new possumwood::CGALPolyhedron(*inMesh));

		switch(algorithmId) {
			case 1:
				SMS::edge_collapse(*mesh, SMS::Count_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));
				break;

			case 2:
				SMS::edge_collapse(*mesh, SMS::Count_ratio_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));
				break;

			case 3:
				SMS::edge_collapse(*mesh, SMS::Edge_length_stop_predicate<float>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));
				break;
		}

		data.set(a_outMesh, std::shared_ptr<const possumwood::CGALPolyhedron>(mesh.release()));
	}
	else
		data.set(a_outMesh, std::shared_ptr<const possumwood::CGALPolyhedron>());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "input");
	meta.addAttribute(
	    a_stopCondition, "stop_condition",
	    possumwood::Enum({std::make_pair("Count_stop_predicate", 1), std::make_pair("Count_ratio_stop_predicate", 2),
	                      std::make_pair("Edge_length_stop_predicate", 3)}));
	meta.addAttribute(a_stopParam, "stop_parameter", 1000.0f);
	meta.addAttribute(a_outMesh, "output");

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_stopCondition, a_outMesh);
	meta.addInfluence(a_stopParam, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/simplification/simplification", init);
}
