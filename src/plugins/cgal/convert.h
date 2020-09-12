#pragma once

namespace possumwood {

template <class SourcePolyhedron, class TargetPolyhedron>
class ConvertPolyhedron : public CGAL::Modifier_base<typename TargetPolyhedron::HalfedgeDS> {
  public:
	typedef typename SourcePolyhedron::Traits SourceKernel;
	typedef typename TargetPolyhedron::Traits TargetKernel;

	typedef typename SourcePolyhedron::HalfedgeDS SourceHDS;
	typedef typename TargetPolyhedron::HalfedgeDS TargetHDS;

	ConvertPolyhedron(const SourcePolyhedron* polyhedron) : m_source(polyhedron) {
	}

	void operator()(TargetHDS& hds) {
		CGAL::Polyhedron_incremental_builder_3<TargetHDS> builder(hds, true);

		builder.begin_surface(m_source->size_of_vertices(), m_source->size_of_facets());

		// need to map from vertices to indices - do that here
		std::unordered_map<typename SourcePolyhedron::Vertex_handle, std::size_t> v2i;
		for(auto v : CGAL::vertices(*m_source)) {
			std::size_t id = v2i.size();
			v2i[v] = id;

			const auto& p = v->point();
			builder.add_vertex(typename TargetKernel::Point_3(p.x(), p.y(), p.z()));
		}

		// and finally need to create the faces array
		for(auto fit : CGAL::faces(*m_source)) {
			builder.begin_facet();

			for(auto v : CGAL::vertices_around_face(CGAL::halfedge(fit, *m_source), *m_source))
				builder.add_vertex_to_facet(v2i[v]);

			builder.end_facet();
		}

		builder.end_surface();
	}

  private:
	const SourcePolyhedron* m_source;
};

template <class SourcePolyhedron, class TargetPolyhedron>
void convert(const SourcePolyhedron& source, TargetPolyhedron& target) {
	ConvertPolyhedron<SourcePolyhedron, TargetPolyhedron> converter(&source);
	target.delegate(converter);
}

}  // namespace possumwood
