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

	vd->addVBO<float, 3>(
		"position",
		4, 1,
		possumwood::VertexData::kStatic,
		[](possumwood::Buffer<float, 3>& buffer) {
			buffer[0][0] = Imath::V3f(-1,-1,1);
			buffer[1][0] = Imath::V3f(1,-1,1);
			buffer[2][0] = Imath::V3f(1,1,1);
			buffer[3][0] = Imath::V3f(-1,1,1);
		}
	);

	vd->addVBO<double, 3>(
		"iNearPositionVert",
		4, 1,
		possumwood::VertexData::kPerDraw,
		[](possumwood::Buffer<double, 3>& buffer) {
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			double modelview[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

			double projection[16];
			glGetDoublev(GL_PROJECTION_MATRIX, projection);

			// points on the near plane, corresponding to each fragment (useful for raytracing)
			gluUnProject(0, 0, 0, modelview, projection, viewport, &buffer[0][0][0], &buffer[0][0][1], &buffer[0][0][2]);
			gluUnProject(viewport[2], 0, 0, modelview, projection, viewport, &buffer[1][0][0], &buffer[1][0][1], &buffer[1][0][2]);
			gluUnProject(viewport[2], viewport[3], 0, modelview, projection, viewport, &buffer[2][0][0], &buffer[2][0][1], &buffer[2][0][2]);
			gluUnProject(0, viewport[3], 0, modelview, projection, viewport, &buffer[3][0][0], &buffer[3][0][1], &buffer[3][0][2]);
		}
	);

	vd->addVBO<double, 3>(
		"iFarPositionVert",
		4, 1,
		possumwood::VertexData::kPerDraw,
		[](possumwood::Buffer<double, 3>& buffer) {
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			double modelview[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

			double projection[16];
			glGetDoublev(GL_PROJECTION_MATRIX, projection);

			// points on the near plane, corresponding to each fragment (useful for raytracing)
			gluUnProject(0, 0, 1, modelview, projection, viewport, &buffer[0][0][0], &buffer[0][0][1], &buffer[0][0][2]);
			gluUnProject(viewport[2], 0, 1, modelview, projection, viewport, &buffer[1][0][0], &buffer[1][0][1], &buffer[1][0][2]);
			gluUnProject(viewport[2], viewport[3], 1, modelview, projection, viewport, &buffer[2][0][0], &buffer[2][0][1], &buffer[2][0][2]);
			gluUnProject(0, viewport[3], 1, modelview, projection, viewport, &buffer[3][0][0], &buffer[3][0][1], &buffer[3][0][2]);
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
