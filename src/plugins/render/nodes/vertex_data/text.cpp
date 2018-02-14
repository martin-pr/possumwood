#include <possumwood_sdk/node_implementation.h>

#include <boost/filesystem.hpp>

#include "datatypes/vertex_data.inl"
#include "datatypes/font.h"

#include <possumwood_sdk/datatypes/filename.h>

namespace {

dependency_graph::InAttr<std::string> a_text;
dependency_graph::InAttr<possumwood::Font> a_font;

dependency_graph::OutAttr<std::shared_ptr<const possumwood::VertexData>> a_vd;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Font& font = data.get(a_font);

	// and render the text into a new VertexData instance
	std::unique_ptr<possumwood::VertexData> vd(new possumwood::VertexData(GL_TRIANGLES));
	{
		// construct the array of vertices
		std::vector<Imath::V2f> vertices;
		std::vector<Imath::V2f> uvs;

		float currentPos = 0;
		for(auto& c : data.get(a_text)) {
			auto& g = font.glyph(c);

			Imath::V2f p1(currentPos*font.size() - g.originX, g.originY - g.height);
			Imath::V2f p2(currentPos*font.size() - g.originX, g.originY);
			Imath::V2f p3(currentPos*font.size() - g.originX + g.width, g.originY);
			Imath::V2f p4(currentPos*font.size() - g.originX + g.width, g.originY - g.height);

			Imath::V2f uv1((float)g.x / font.width(), (float)(g.y + g.height) / font.height());
			Imath::V2f uv2((float)g.x / font.width(), (float)g.y / font.height());
			Imath::V2f uv3((float)(g.x + g.width) / font.width(), (float)g.y / font.height());
			Imath::V2f uv4((float)(g.x + g.width) / font.width(), (float)(g.y + g.height) / font.height());

			vertices.push_back(p1 / font.size());
			vertices.push_back(p2 / font.size());
			vertices.push_back(p3 / font.size());

			vertices.push_back(p1 / font.size());
			vertices.push_back(p3 / font.size());
			vertices.push_back(p4 / font.size());

			uvs.push_back(uv1);
			uvs.push_back(uv2);
			uvs.push_back(uv3);

			uvs.push_back(uv1);
			uvs.push_back(uv3);
			uvs.push_back(uv4);

			currentPos += (float)g.advance / font.size();
		}

		// and add them to the vertex data
		vd->addVBO<Imath::V2f>("position", vertices.size(), possumwood::VertexData::kStatic,
			[vertices](possumwood::Buffer<float>& buffer, const possumwood::Drawable::ViewportState& viewport) {
				std::size_t ctr = 0;
				for(auto& v : vertices) {
					buffer.element(ctr) = v;
					++ctr;
				}
			}
		);

		vd->addVBO<Imath::V2f>("uv", uvs.size(), possumwood::VertexData::kStatic,
			[uvs](possumwood::Buffer<float>& buffer, const possumwood::Drawable::ViewportState& viewport) {
				std::size_t ctr = 0;
				for(auto& v : uvs) {
					buffer.element(ctr) = v;
					++ctr;
				}
			}
		);
	}

	data.set(a_vd, std::shared_ptr<const possumwood::VertexData>(vd.release()));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_text, "text");
	meta.addAttribute(a_font, "font");
	meta.addAttribute(a_vd, "vertex_data");

	meta.addInfluence(a_font, a_vd);
	meta.addInfluence(a_text, a_vd);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("render/vertex_data/text", init);

}
