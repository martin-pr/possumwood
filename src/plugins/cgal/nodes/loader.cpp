#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <fstream>

#include <CGAL/IO/OBJ_reader.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>

#include "datatypes/polyhedron.h"
#include "cgal.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::CGALPolyhedron>> a_polyhedron;

class Builder : public CGAL::Modifier_base<possumwood::CGALPolyhedron::HalfedgeDS> {
  public:
	Builder(const std::vector<possumwood::CGALPolyhedron::Point>* points,
	        const std::vector<std::vector<std::size_t>>* faces)
	    : m_points(points), m_faces(faces) {
	}

	void operator()(possumwood::CGALPolyhedron::HalfedgeDS& hds) {
		CGAL::Polyhedron_incremental_builder_3<possumwood::CGALPolyhedron::HalfedgeDS> B(hds, true);

		B.begin_surface(m_points->size(), m_faces->size());

		for(auto& p : *m_points)
			B.add_vertex(p);

		for(auto& f : *m_faces) {
			B.begin_facet();

			for(auto& vi : f)
				B.add_vertex_to_facet(vi);

			B.end_facet();
		}

		B.end_surface();
	}

  private:
	const std::vector<possumwood::CGALPolyhedron::Point>* m_points;
	const std::vector<std::vector<std::size_t>>* m_faces;
};

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	std::vector<possumwood::CGALPolyhedron::Point> points;
	std::vector<std::vector<std::size_t>> faces;

	std::ifstream file(filename.filename().string().c_str());
	bool result = CGAL::read_OBJ(file, points, faces);

	if(!result)
		throw std::runtime_error("Error reading file '" + filename.filename().string() + "'");

	std::unique_ptr<possumwood::CGALPolyhedron> polyhedron(new possumwood::CGALPolyhedron());

	{
		Builder builder(&points, &faces);
		polyhedron->delegate(builder);
	}

	data.set(a_polyhedron, std::shared_ptr<const possumwood::CGALPolyhedron>(polyhedron.release()));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
	                                              "OBJ files (*.obj)",
	                                          }));
	meta.addAttribute(a_polyhedron, "polyhedron");

	meta.addInfluence(a_filename, a_polyhedron);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/loader", init);
}
