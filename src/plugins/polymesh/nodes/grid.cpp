#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <fstream>

#include "generic_polymesh.inl"
#include "traits.h"
#include "maths/io/vec3.h"

namespace {

dependency_graph::InAttr<unsigned> a_xSubd, a_ySubd;
dependency_graph::InAttr<float> a_xSize, a_ySize;
dependency_graph::InAttr<Imath::V3f> a_origin;
dependency_graph::OutAttr<possumwood::polymesh::GenericPolymesh> a_polymesh;

dependency_graph::State compute(dependency_graph::Values& data) {
	if(data.get(a_xSubd) < 1 || data.get(a_ySubd) < 1)
		throw std::runtime_error("X and Y subdivision values have to be larger than 0");

	possumwood::polymesh::GenericPolymesh mesh;

	auto p_handle = mesh.vertices().handles().add("P", std::array<float, 3>());

	// generate vertices
	for(unsigned y=0;y<=data.get(a_ySubd);++y) {
		const float yf = data.get(a_origin).y + (float)y / (float)(data.get(a_ySubd)) * data.get(a_ySize);
		for(unsigned x=0;x<=data.get(a_xSubd);++x) {
			const float xf = data.get(a_origin).x + (float)x / (float)(data.get(a_xSubd)) * data.get(a_xSize);

			auto it = mesh.vertices().add();
			it->set(p_handle, std::array<float, 3>({{xf, yf, data.get(a_origin).z}}));
		}
	}

	// generate polygons
	for(unsigned y=0;y<data.get(a_ySubd);++y)
		for(unsigned x=0;x<data.get(a_xSubd);++x) {
			const std::array<std::size_t, 4> arr{{
				x+y*(data.get(a_xSubd)+1),
				(x+1)+y*(data.get(a_xSubd)+1),
				(x+1)+(y+1)*(data.get(a_xSubd)+1),
				x+(y+1)*(data.get(a_xSubd)+1)
			}};

			mesh.polygons().add(arr.begin(), arr.end());
		}

	data.set(a_polymesh, mesh);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_xSubd, "subdivision/x", 1u);
	meta.addAttribute(a_ySubd, "subdivision/y", 1u);
	meta.addAttribute(a_xSize, "size/x", 1.0f);
	meta.addAttribute(a_ySize, "size/y", 1.0f);
	meta.addAttribute(a_origin, "origin", Imath::V3f(0,0,0));
	meta.addAttribute(a_polymesh, "polymesh");

	meta.addInfluence(a_xSubd, a_polymesh);
	meta.addInfluence(a_ySubd, a_polymesh);
	meta.addInfluence(a_xSize, a_polymesh);
	meta.addInfluence(a_ySize, a_polymesh);
	meta.addInfluence(a_origin, a_polymesh);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("polymesh/grid", init);
}
