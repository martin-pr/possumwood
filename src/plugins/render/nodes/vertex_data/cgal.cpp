#include "cgal/datatypes/meshes.h"

#include <boost/algorithm/string/join.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathVec.h>

#include "datatypes/vertex_data.inl"

namespace possumwood {

template <typename T>
struct VBOTraits<CGAL::Point_3<CGAL::Simple_cartesian<T>>> {
	typedef T element;
	static constexpr std::size_t width() {
		return 3;
	};
};

template <typename T>
struct VBOTraits<std::vector<T>> {
	typedef T element;
	static constexpr std::size_t width() {
		return 1;
	};
};
}

namespace {

template <typename T>
T defaultExtract(const T& val) {
	return val;
}

void addVerticesVBO(possumwood::VertexData& vd, const possumwood::Meshes& meshes, std::size_t triangleCount) {
	vd.addVBO<possumwood::CGALPolyhedron::Point_3>(
	"position", triangleCount*3, possumwood::VertexData::kStatic,
	[meshes, triangleCount](possumwood::Buffer<typename possumwood::VBOTraits<possumwood::CGALPolyhedron::Point_3>::element>& buffer,
 	        const possumwood::Drawable::ViewportState& vs) {

		// TODO: use index buffer instead
		std::size_t index = 0;

		for(const auto& m : meshes) {
			for(auto fit = m.polyhedron().facets_begin(); fit != m.polyhedron().facets_end(); ++fit)
				if(fit->facet_degree() > 2) {
					auto vit = fit->facet_begin();

					const auto& p1 = vit->vertex()->point();
					++vit;

					const auto& p2 = vit->vertex()->point();
					++vit;

					for(std::size_t v = 2; v < fit->facet_degree(); ++v) {
					    const auto& p = vit->vertex()->point();

					    buffer.element(index++) = p1;
					    buffer.element(index++) = p2;
					    buffer.element(index++) = p;

						++vit;
					}
				}
		}

		assert(index == triangleCount*3);
	});
}

using possumwood::Meshes;
using possumwood::CGALPolyhedron;

dependency_graph::OutAttr<std::shared_ptr<const possumwood::VertexData>> a_vd;
dependency_graph::InAttr<Meshes> a_mesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const Meshes mesh = data.get(a_mesh);

	// we're drawing triangles
	std::unique_ptr<possumwood::VertexData> vd(new possumwood::VertexData(GL_TRIANGLES));

	// first, figure out how many triangles we have
	//   there has to be a better way to do this, come on
	std::size_t triangleCount = 0;
	for(auto& m : mesh)
		for(auto it = m.polyhedron().facets_begin(); it != m.polyhedron().facets_end(); ++it)
			if(it->facet_degree() > 2)
				triangleCount += (it->facet_degree() - 2);

	// and build the buffers
	if(triangleCount > 0) {
		addVerticesVBO(*vd, mesh, triangleCount);
	}

	data.set(a_vd, std::shared_ptr<const possumwood::VertexData>(vd.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_vd, "vertex_data");
	meta.addAttribute(a_mesh, "mesh");

	meta.addInfluence(a_mesh, a_vd);

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/vertex_data/cgal", init);
}
