#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glu.h>

#include <ImathVec.h>

#include "datatypes/vertex_data.inl"

namespace {

dependency_graph::OutAttr<std::shared_ptr<const possumwood::VertexData>> a_vd;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	// we're drawing quads
	std::unique_ptr<possumwood::VertexData> vd(new possumwood::VertexData(GL_QUADS));

	vd->addVBO<Imath::V3f>(
		"position",
		4,
		possumwood::VertexData::kStatic,
		[](possumwood::Buffer<float>& buffer, const possumwood::Drawable::ViewportState& viewport) {
			buffer.element(0) = Imath::V3f(-1,-1,1);
			buffer.element(1) = Imath::V3f(1,-1,1);
			buffer.element(2) = Imath::V3f(1,1,1);
			buffer.element(3) = Imath::V3f(-1,1,1);
		}
	);

	vd->addVBO<Imath::V3d>(
		"iNearPositionVert",
		4,
		possumwood::VertexData::kPerDraw,
		[](possumwood::Buffer<double>& buffer, const possumwood::Drawable::ViewportState& vp) {
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			double modelview[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

			double projection[16];
			glGetDoublev(GL_PROJECTION_MATRIX, projection);

			// points on the near plane, corresponding to each fragment (useful for raytracing)
			double* ptr = buffer.element(0).ptr();
			gluUnProject(0, 0, 0, modelview, projection, viewport, ptr, ptr+1, ptr+2);
			gluUnProject(viewport[2], 0, 0, modelview, projection, viewport, ptr+3, ptr+4, ptr+5);
			gluUnProject(viewport[2], viewport[3], 0, modelview, projection, viewport, ptr+6, ptr+7, ptr+8);
			gluUnProject(0, viewport[3], 0, modelview, projection, viewport, ptr+9, ptr+10, ptr+11);
		}
	);

	vd->addVBO<Imath::V3d>(
		"iFarPositionVert",
		4,
		possumwood::VertexData::kPerDraw,
		[](possumwood::Buffer<double>& buffer, const possumwood::Drawable::ViewportState& vp) {
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			double modelview[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

			double projection[16];
			glGetDoublev(GL_PROJECTION_MATRIX, projection);

			// points on the near plane, corresponding to each fragment (useful for raytracing)
			double* ptr = buffer.element(0).ptr();
			gluUnProject(0, 0, 1, modelview, projection, viewport, ptr, ptr+1, ptr+2);
			gluUnProject(viewport[2], 0, 1, modelview, projection, viewport, ptr+3, ptr+4, ptr+5);
			gluUnProject(viewport[2], viewport[3], 1, modelview, projection, viewport, ptr+6, ptr+7, ptr+8);
			gluUnProject(0, viewport[3], 1, modelview, projection, viewport, ptr+9, ptr+10, ptr+11);
		}
	);

	data.set(a_vd, std::shared_ptr<const possumwood::VertexData>(vd.release()));

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_vd, "vertex_data");

	meta.setCompute(&compute);
}

possumwood::NodeImplementation s_impl("render/vertex_data/background", init);

}
