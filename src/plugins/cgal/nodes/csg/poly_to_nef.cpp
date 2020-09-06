#include <unordered_map>

#include <CGAL/Nef_polyhedron_3.h>

#include <possumwood_sdk/node_implementation.h>

#include "cgal.h"
#include "datatypes/meshes.h"
#include "errors.h"

namespace {

using possumwood::CGALPolyhedron;
using possumwood::Meshes;

dependency_graph::InAttr<Meshes> a_polymesh;
dependency_graph::OutAttr<possumwood::CGALNefPolyhedron> a_nef;

namespace {

template <class HDS>
class ConvertPolyhedron : public CGAL::Modifier_base<HDS> {
  public:
	ConvertPolyhedron(const possumwood::Mesh& mesh) : m_mesh(mesh) {
	}
	void operator()(HDS& hds) {
		CGAL::Polyhedron_incremental_builder_3<CGAL::Polyhedron_3<possumwood::NefKernel>::HalfedgeDS> builder(hds,
		                                                                                                      true);

		builder.begin_surface(m_mesh.polyhedron().size_of_vertices(), m_mesh.polyhedron().size_of_facets());

		// need to map from vertices to indices - do that here
		std::unordered_map<CGALPolyhedron::Vertex_handle, std::size_t> v2i;
		for(auto v : CGAL::vertices(m_mesh.polyhedron())) {
			std::size_t id = v2i.size();
			v2i[v] = id;

			const auto& p = v->point();
			builder.add_vertex(possumwood::NefKernel::Point_3(p.x(), p.y(), p.z()));
		}

		// and finally need to create the faces array
		for(auto fit : CGAL::faces(m_mesh.polyhedron())) {
			builder.begin_facet();

			for(auto v : CGAL::vertices_around_face(CGAL::halfedge(fit, m_mesh.polyhedron()), m_mesh.polyhedron()))
				builder.add_vertex_to_facet(v2i[v]);

			builder.end_facet();
		}

		builder.end_surface();
	}

  private:
	possumwood::Mesh m_mesh;
};

}  // namespace

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State state;

	possumwood::ScopedOutputRedirect outRedirect;

	try {
		possumwood::CGALNef result;

		for(auto& mesh : data.get(a_polymesh)) {
			// convert the data to a compatible kernel type
			CGAL::Polyhedron_3<possumwood::NefKernel> poly;

			ConvertPolyhedron<CGAL::Polyhedron_3<possumwood::NefKernel>::HalfedgeDS> converter(mesh);
			poly.delegate(converter);

			if(!poly.is_closed())
				throw std::runtime_error("Polymesh " + mesh.name() + " is not closed!");

			result = possumwood::CGALNef(poly);
		}

		data.set(a_nef,
		         possumwood::CGALNefPolyhedron(std::unique_ptr<possumwood::CGALNef>(new possumwood::CGALNef(result))));

		// any warnings / errors that were not converted to exceptions by CGAL
		state.append(outRedirect.state());
	} catch(const std::exception& err) {
		// usually, the output printouts in CGAL are better than the exceptions - let's use those first
		state.append(outRedirect.state());
		// and add the exception content
		state.addError(err.what());
	}

	return state;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_polymesh, "mesh", possumwood::Meshes(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_nef, "nef", possumwood::CGALNefPolyhedron(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_polymesh, a_nef);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("cgal/csg/poly_to_nef", init);
}  // namespace
