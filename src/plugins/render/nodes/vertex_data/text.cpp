#include <possumwood_sdk/node_implementation.h>

#include <boost/filesystem.hpp>

#include "datatypes/vertex_data.inl"
#include "datatypes/font.h"

#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/datatypes/enum.h>

namespace {

dependency_graph::InAttr<std::string> a_text;
dependency_graph::InAttr<possumwood::Font> a_font;
dependency_graph::InAttr<possumwood::Enum> a_horizAlign, a_vertAlign;

dependency_graph::OutAttr<possumwood::VertexData> a_vd;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Font& font = data.get(a_font);

	const int horizAlign = data.get(a_horizAlign).intValue();
	const int vertAlign = data.get(a_vertAlign).intValue();

	// and render the text into a new VertexData instance
	possumwood::VertexData vd(GL_TRIANGLES);
	if(!data.get(a_text).empty()){
		// first, figure out the size of the text
		float totalWidth = 0.0f;
		for(auto& c : data.get(a_text)) {
			auto& g = font.glyph(c);
			totalWidth += (float)g.advance / font.size();
		}

		// construct the array of vertices
		std::vector<Imath::V2f> vertices;
		std::vector<Imath::V2f> uvs;

		float currentPos = 0;
		float vertPos = 0;

		if(horizAlign == 1)
			currentPos = -totalWidth / 2.0f;
		else if(horizAlign == 2)
			currentPos = 0;
		else
			currentPos = -totalWidth;

		if(vertAlign == 1)
			vertPos = -font.size() / 2.0f;
		else if(vertAlign == 2)
			vertPos = -font.size();
		else
			vertPos = 0;

		for(auto& c : data.get(a_text)) {
			auto& g = font.glyph(c);

			Imath::V2f p1(currentPos * font.size() - g.originX, g.originY - g.height + vertPos);
			Imath::V2f p2(currentPos * font.size() - g.originX, g.originY + vertPos);
			Imath::V2f p3(currentPos * font.size() - g.originX + g.width, g.originY + vertPos);
			Imath::V2f p4(currentPos * font.size() - g.originX + g.width, g.originY - g.height + vertPos);

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
		vd.addVBO<Imath::V2f>("position", vertices.size(), possumwood::VertexData::kStatic,
		[vertices](possumwood::Buffer<float>& buffer, const possumwood::ViewportState & viewport) {
			std::size_t ctr = 0;
			for(auto& v : vertices) {
				buffer.element(ctr) = v;
				++ctr;
			}
		}
		                      );

		vd.addVBO<Imath::V2f>("uv", uvs.size(), possumwood::VertexData::kStatic,
		[uvs](possumwood::Buffer<float>& buffer, const possumwood::ViewportState & viewport) {
			std::size_t ctr = 0;
			for(auto& v : uvs) {
				buffer.element(ctr) = v;
				++ctr;
			}
		}
		                      );
	}

	data.set(a_vd, std::move(vd));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_text, "text");
	meta.addAttribute(a_font, "font");
	meta.addAttribute(a_vd, "vertex_data");

	meta.addAttribute(
	    a_horizAlign, "horiz_align",
	    possumwood::Enum({std::make_pair("Center", 1),
	                      std::make_pair("Left", 2),
	                      std::make_pair("Right", 3)
	                     }));

	meta.addAttribute(
	    a_vertAlign, "vert_align",
	    possumwood::Enum({std::make_pair("Center", 1),
	                      std::make_pair("Top", 2),
	                      std::make_pair("Bottom", 3)
	                     }));

	meta.addInfluence(a_font, a_vd);
	meta.addInfluence(a_text, a_vd);
	meta.addInfluence(a_horizAlign, a_vd);
	meta.addInfluence(a_vertAlign, a_vd);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("render/vertex_data/text", init);

}
