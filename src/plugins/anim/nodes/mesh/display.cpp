#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

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

struct Color {
	float r, g, b;

	Color operator * (float w) const {
		return Color{r*w, g*w, b*w};
	}

	Color& operator += (const Color& c) {
		r += c.r;
		g += c.g;
		b += c.b;

		return *this;
	}

	operator float*() {
		return &r;
	}
};

Color color(unsigned index) {
	static std::vector<Color> s_colors;
	while(s_colors.size() <= index)
		s_colors.push_back(Color{
			halton(s_colors.size(), 2),
			halton(s_colors.size(), 3),
			halton(s_colors.size(), 5)
		});

	return s_colors[index];
};

Color makeColor(const anim::SkinnedVertices::Vertex& v) {
	Color result{0,0,0};

	for(auto& w : v)
		result += color(w.first) * w.second;

	return result;
}

dependency_graph::InAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_meshes;
dependency_graph::InAttr<bool> a_colorBones;

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
	}

	void draw() {
		std::shared_ptr<const std::vector<anim::SkinnedMesh>> meshes = values().get(a_meshes);
		bool colorBones = values().get(a_colorBones);

		if(meshes != nullptr) {
			glColor3f(1, 1, 1);

			glPushAttrib(GL_ALL_ATTRIB_BITS);

			glEnable(GL_LIGHTING);
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
			float black[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);

			if(colorBones) {
				glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
				glEnable(GL_COLOR_MATERIAL);
			}

			glBegin(GL_TRIANGLES);

			for(auto& mesh : *meshes) {
				std::size_t ctr = 0;
				for(auto& poly : mesh.polygons()) {
					const anim::SkinnedVertices::Vertex& v1 = mesh.vertices()[poly[0]];
					const anim::SkinnedVertices::Vertex& v2 = mesh.vertices()[poly[1]];
					const anim::SkinnedVertices::Vertex& v3 = mesh.vertices()[poly[2]];

					const Imath::V3f& v1p = v1.pos();
					const Imath::V3f& v2p = v2.pos();
					const Imath::V3f& v3p = v3.pos();

					// const Imath::V3f norm = (v2p-v1p).cross(v3p-v1p).normalized();
					// glNormal3fv(&norm[0]);

					if(colorBones)
						glColor3fv(makeColor(v1));
					glNormal3fv(&mesh.normals()[ctr++][0]);
					glVertex3fv(&v1p[0]);

					if(colorBones)
						glColor3fv(makeColor(v2));
					glNormal3fv(&mesh.normals()[ctr++][0]);
					glVertex3fv(&v2p[0]);

					if(colorBones)
						glColor3fv(makeColor(v3));
					glNormal3fv(&mesh.normals()[ctr++][0]);
					glVertex3fv(&v3p[0]);
				}
			}

			glEnd();

			glPopAttrib();
		}
	}
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_meshes, "meshes");
	meta.addAttribute(a_colorBones, "color_bones", false);

	meta.setDrawable<Drawable>();
}

possumwood::NodeImplementation s_impl("anim/mesh/display", init);

}
