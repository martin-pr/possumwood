#include <possumwood_sdk/node_implementation.h>

#include <CGAL/squared_distance_3.h>

#include <CGAL/Surface_mesh_simplification/edge_collapse.h>

#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_stop_predicate.h>

#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_cost.h>

#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_placement.h>
// #include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Constrained_placement.h>
// #include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Bounded_normal_change_placement.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "datatypes/meshes.h"

#include "cgal.h"

namespace SMS = CGAL::Surface_mesh_simplification;
using namespace std::placeholders;

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

namespace {

dependency_graph::InAttr<Meshes> a_inMesh;
dependency_graph::InAttr<float> a_stopParam;
dependency_graph::InAttr<possumwood::Enum> a_stopCondition, a_cost, a_placement;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Meshes& inMeshes = data.get(a_inMesh);
	const float stopCondition = data.get(a_stopParam);
	const unsigned algorithmId =
	    data.get(a_stopCondition).intValue() + data.get(a_cost).intValue() + data.get(a_placement).intValue();

	Meshes result;

	for(auto inMesh : inMeshes) {
		std::unique_ptr<possumwood::CGALPolyhedron> mesh(new possumwood::CGALPolyhedron(inMesh.mesh()));

		// this is just horrible - need to find a better way
		switch(algorithmId) {
			case 111:
				SMS::edge_collapse(*mesh, SMS::Count_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));
				break;

			case 112:
				SMS::edge_collapse(*mesh, SMS::Count_ratio_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));
				break;

			case 113:
				SMS::edge_collapse(*mesh, SMS::Edge_length_stop_predicate<float>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));
				break;
			case 121:
				SMS::edge_collapse(*mesh, SMS::Count_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::LindstromTurk_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));
				break;

			case 122:
				SMS::edge_collapse(*mesh, SMS::Count_ratio_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::LindstromTurk_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));
				break;

			case 123:
				SMS::edge_collapse(*mesh, SMS::Edge_length_stop_predicate<float>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::LindstromTurk_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::Midpoint_placement<possumwood::CGALPolyhedron>()));
				break;

			case 211:
				SMS::edge_collapse(*mesh, SMS::Count_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::LindstromTurk_placement<possumwood::CGALPolyhedron>()));
				break;

			case 212:
				SMS::edge_collapse(*mesh, SMS::Count_ratio_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::LindstromTurk_placement<possumwood::CGALPolyhedron>()));
				break;

			case 213:
				SMS::edge_collapse(*mesh, SMS::Edge_length_stop_predicate<float>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::Edge_length_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::LindstromTurk_placement<possumwood::CGALPolyhedron>()));
				break;
			case 221:
				SMS::edge_collapse(*mesh, SMS::Count_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::LindstromTurk_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::LindstromTurk_placement<possumwood::CGALPolyhedron>()));
				break;

			case 222:
				SMS::edge_collapse(*mesh, SMS::Count_ratio_stop_predicate<possumwood::CGALPolyhedron>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::LindstromTurk_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::LindstromTurk_placement<possumwood::CGALPolyhedron>()));
				break;

			case 223:
				SMS::edge_collapse(*mesh, SMS::Edge_length_stop_predicate<float>(stopCondition),
				                   CGAL::parameters::get_cost(SMS::LindstromTurk_cost<possumwood::CGALPolyhedron>())
				                       .get_placement(SMS::LindstromTurk_placement<possumwood::CGALPolyhedron>()));
				break;
		}

		mesh->collect_garbage();

		result.addMesh(inMesh.name(), std::move(mesh));
	}

	data.set(a_outMesh, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "input");
	meta.addAttribute(
	    a_stopCondition, "stop_condition",
	    possumwood::Enum({std::make_pair("Count_stop_predicate", 1), std::make_pair("Count_ratio_stop_predicate", 2),
	                      std::make_pair("Edge_length_stop_predicate", 3)}));
	meta.addAttribute(a_cost, "cost", possumwood::Enum({std::make_pair("Edge_length_cost", 10),
	                                                    std::make_pair("LindstromTurk_cost", 20)}));
	meta.addAttribute(a_placement, "placement", possumwood::Enum({std::make_pair("Midpoint_placement", 100),
	                                                              std::make_pair("LindstromTurk_placement", 200)}));
	meta.addAttribute(a_stopParam, "stop_parameter", 1000.0f);
	meta.addAttribute(a_outMesh, "output");

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_stopCondition, a_outMesh);
	meta.addInfluence(a_cost, a_outMesh);
	meta.addInfluence(a_placement, a_outMesh);
	meta.addInfluence(a_stopParam, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/simplification/simplification", init);
}
