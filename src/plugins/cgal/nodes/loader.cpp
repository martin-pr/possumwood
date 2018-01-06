#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <fstream>

#include <CGAL/IO/OBJ_reader.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>

#include "datatypes/meshes.h"
#include "meshes.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::InAttr<std::string> a_name;
dependency_graph::OutAttr<possumwood::Meshes> a_polyhedron;

template <class HDS, class POINTS, class FACES>
class Builder : public CGAL::Modifier_base<HDS> {
  public:
	Builder(const POINTS& pts, const FACES& f) : m_points(&pts), m_faces(&f) {
	}
	void operator()(HDS& hds) {
		// Postcondition: hds is a valid polyhedral surface.
		CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);

		B.begin_surface(m_points->size(), m_faces->size());

		for(auto& v : *m_points)
			B.add_vertex(v);

		for(auto& f : *m_faces) {
			B.begin_facet();

			for(auto& i : f)
				B.add_vertex_to_facet(i);

			B.end_facet();
		}

		B.end_surface();
	}

  private:
	const POINTS* m_points;
	const FACES* m_faces;
};

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	std::vector<possumwood::CGALKernel::Point_3> points;
	std::vector<std::vector<std::size_t>> faces;

	std::ifstream file(filename.filename().string().c_str());
	bool result = CGAL::read_OBJ(file, points, faces);

	if(!result)
		throw std::runtime_error("Error reading file '" + filename.filename().string() +
		                         "'");

	std::unique_ptr<possumwood::CGALPolyhedron> polyhedron(
	    new possumwood::CGALPolyhedron());

	Builder<possumwood::CGALPolyhedron::HalfedgeDS, typeof(points), typeof(faces)>
	    builder(points, faces);
	polyhedron->delegate(builder);

	possumwood::Meshes out;
	out.addMesh(data.get(a_name), std::move(polyhedron));

	data.set(a_polyhedron, out);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
	                                              "OBJ files (*.obj)",
	                                          }));
	meta.addAttribute(a_name, "name", std::string("mesh"));
	meta.addAttribute(a_polyhedron, "polyhedron");

	meta.addInfluence(a_name, a_polyhedron);
	meta.addInfluence(a_filename, a_polyhedron);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/loader", init);
}
