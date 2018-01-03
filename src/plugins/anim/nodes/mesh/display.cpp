#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/gl_renderable.h>

#include <GL/glut.h>

#include "datatypes/skinned_mesh.h"

namespace {

float halton(std::size_t index, std::size_t base) {
	float result = 0.0f;

	float f = 1.0f;

	while(index > 0) {
		f /= (float)base;
		result += f * (float)(index % base);
		index /= base;
	}

	return result;
}

Imath::V3f color(unsigned index) {
	static std::vector<Imath::V3f> s_colors;
	while(s_colors.size() <= index)
		s_colors.push_back(Imath::V3f{halton(s_colors.size(), 2),
		                              halton(s_colors.size(), 3),
		                              halton(s_colors.size(), 5)});

	return s_colors[index];
};

Imath::V3f makeColor(const anim::SkinnedVertices::Vertex& v) {
	Imath::V3f result{0, 0, 0};

	for(auto& w : v.skinning())
		result += color(w.bone) * w.weight;

	return result;
}

const std::string& vertexShaderSource() {
	static const std::string s_source =
	    " \
		#version 150 \n \
		\n \
		uniform mat4 in_Projection; \n \
		uniform mat4 in_Modelview; \n \
		\n \
	    in vec3 position; \n \
	    in vec3 normal; \n \
	    in vec3 color; \n \
	    \n \
		out vec3 vert_color; \n \
		out vec3 vert_normal; \n \
		\n \
		void main(void) { \n \
		    gl_Position = in_Projection * in_Modelview * vec4(position, 1.0); \n \
			vert_color = color; \n \
			vert_normal = (in_Modelview * vec4(normal, 0.0)).xyz; \n \
		}";

	return s_source;
}

const std::string& fragmentShaderSource() {
	static const std::string s_source =
	    " \
		#version 150 \n \
		\n \
		in vec3 vert_color; \n \
		in vec3 vert_normal; \n \
		\n \
		out vec4 out_color; \n \
		\n \
		void main(void) { \n \
			vec3 norm = normalize(vert_normal); \n \
		    out_color = vec4(vert_color, 1.0) * vert_normal.z; \n \
		}";

	return s_source;
}

dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_meshes;
dependency_graph::InAttr<bool> a_colorBones;

class Drawable : public possumwood::Drawable {
  public:
	Drawable(dependency_graph::Values&& vals)
	    : possumwood::Drawable(std::move(vals)),
	      m_renderable(GL_TRIANGLES, vertexShaderSource(), fragmentShaderSource()) {
	}

	dependency_graph::State draw() {
		std::shared_ptr<const std::vector<anim::SkinnedMesh>> meshes =
		    values().get(a_meshes);
		bool colorBones = values().get(a_colorBones);

		{
			auto positionVBO = m_renderable.updateVertexData("position");
			positionVBO.data.clear();

			auto normalVBO = m_renderable.updateVertexData("normal");
			normalVBO.data.clear();

			auto colorVBO = m_renderable.updateVertexData("color");
			colorVBO.data.clear();

			if(meshes != nullptr) {
				// glBegin(GL_TRIANGLES);

				for(auto& mesh : *meshes) {
					std::size_t ctr = 0;
					for(auto& poly : mesh.polygons()) {
						const anim::SkinnedVertices::Vertex& v1 =
						    mesh.vertices()[poly[0]];
						const anim::SkinnedVertices::Vertex& v2 =
						    mesh.vertices()[poly[1]];
						const anim::SkinnedVertices::Vertex& v3 =
						    mesh.vertices()[poly[2]];

						const Imath::V3f& v1p = v1.pos();
						const Imath::V3f& v2p = v2.pos();
						const Imath::V3f& v3p = v3.pos();

						if(colorBones)
							colorVBO.data.push_back(makeColor(v1));
						else
							colorVBO.data.push_back(Imath::V3f(1, 1, 1));
						normalVBO.data.push_back(mesh.normals()[ctr++]);
						positionVBO.data.push_back(v1p);

						if(colorBones)
							colorVBO.data.push_back(makeColor(v2));
						else
							colorVBO.data.push_back(Imath::V3f(1, 1, 1));
						normalVBO.data.push_back(mesh.normals()[ctr++]);
						positionVBO.data.push_back(v2p);

						if(colorBones)
							colorVBO.data.push_back(makeColor(v3));
						else
							colorVBO.data.push_back(Imath::V3f(1, 1, 1));
						normalVBO.data.push_back(mesh.normals()[ctr++]);
						positionVBO.data.push_back(v3p);
					}
				}
			}
		}

		m_renderable.draw(viewport().projection, viewport().modelview);

		return dependency_graph::State();
	}

  private:
	possumwood::GLRenderable m_renderable;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_meshes, "meshes");
	meta.addAttribute(a_colorBones, "color_bones", false);

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("anim/mesh/display", init);
}
