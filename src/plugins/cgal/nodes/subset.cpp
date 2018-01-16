#include <possumwood_sdk/node_implementation.h>

#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathEuler.h>

#include <anim/datatypes/subset_selection.h>

#include "maths/io/vec3.h"
#include "datatypes/meshes.h"
#include "cgal.h"

namespace {

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

dependency_graph::InAttr<Meshes> a_inMesh;
dependency_graph::InAttr<anim::SubsetSelection> a_subset;
dependency_graph::OutAttr<Meshes> a_outMesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	const Meshes& input = data.get(a_inMesh);
	const anim::SubsetSelection& selection = data.get(a_subset);

	// update the subset options
	{
		// build the options from the current input
		anim::SubsetSelection::Options options;
		for(auto& m : input)
			options.add(m.name());

		// update a_editorData, if needed
		if(data.get(a_subset).options() != options) {
			anim::SubsetSelection subsetData = data.get(a_subset);
			subsetData.options() = options;
			data.set(a_subset, subsetData);
		}
	}


	Meshes output;

	for(auto& mesh : input) {
		auto it = selection.find(mesh.name());
		if(it != selection.end() && it->second)
			output.addMesh(mesh);
	}

	data.set(a_outMesh, output);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inMesh, "in_mesh");
	meta.addAttribute(a_subset, "subset");
	meta.addAttribute(a_outMesh, "out_mesh");

	meta.addInfluence(a_inMesh, a_outMesh);
	meta.addInfluence(a_subset, a_outMesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/subset", init);
}
