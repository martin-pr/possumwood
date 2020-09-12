#pragma once

#include <unordered_map>

#include "cgal.h"

namespace possumwood {

template <typename DEST>
struct CGALConversion {
	template <typename SRC>
	static DEST conv(const SRC& s) {
		return static_cast<DEST>(s);
	}
};

template <>
struct CGALConversion<float> {
	template <typename T>
	static float conv(const CGAL::Lazy_exact_nt<T>& s) {
		const T tmp = s.exact();
		return tmp.template convert_to<float>();
	}
};

template <typename DEST_KERN>
struct CGALConversion<CGAL::Point_3<DEST_KERN>> {
	template <typename SRC_KERN>
	static CGAL::Point_3<DEST_KERN> conv(const CGAL::Point_3<SRC_KERN>& s) {
		return CGAL::Point_3<DEST_KERN>(CGALConversion<typename DEST_KERN::FT>::conv(s.x()),
		                                CGALConversion<typename DEST_KERN::FT>::conv(s.y()),
		                                CGALConversion<typename DEST_KERN::FT>::conv(s.z()));
	}
};

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

			builder.add_vertex(CGALConversion<typename TargetKernel::Point_3>::conv(v->point()));
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
