#include "setup.h"

namespace possumwood {

GLSetup::ScopedGLSetup::ScopedGLSetup() {
	glGetBooleanv(GL_CULL_FACE, &m_cullFaceEnabled);
	glGetIntegerv(GL_CULL_FACE_MODE, &m_cullFace);
	glGetIntegerv(GL_FRONT_FACE, &m_frontFace);
}

GLSetup::ScopedGLSetup::~ScopedGLSetup() {
	if(m_cullFaceEnabled)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	glCullFace(m_cullFace);
	glFrontFace(m_frontFace);
}

/////////////////////////////////////////////////////////

GLSetup::GLSetup() : m_faceCulling(kCCW) {
}

void GLSetup::setFaceCulling(Culling culling) {
	m_faceCulling = culling;
}

GLSetup::Culling GLSetup::faceCulling() const {
	return m_faceCulling;
}

std::unique_ptr<GLSetup::ScopedGLSetup> GLSetup::apply() const {
	std::unique_ptr<GLSetup::ScopedGLSetup> out(new GLSetup::ScopedGLSetup());

	if(m_faceCulling == kNone)
		glDisable(GL_CULL_FACE);
	else {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		if(m_faceCulling == kCCW)
			glFrontFace(GL_CCW);
		else
			glFrontFace(GL_CW);
	}

	return out;
}

std::ostream& operator<<(std::ostream& out, const GLSetup& setup) {
	out << "Culling: ";
	switch(setup.faceCulling()) {
		case GLSetup::kNone:
			out << "none";
			break;
		case GLSetup::kCCW:
			out << "ccw";
			break;
		case GLSetup::kCW:
			out << "cw";
			break;
	}

	return out;
}

namespace {

void toJson(::nlohmann::json& json, const GLSetup& value) {
	switch(value.faceCulling()) {
		case GLSetup::kNone:
			json["culling"] = "none";
			break;
		case GLSetup::kCCW:
			json["culling"] = "ccw";
			break;
		case GLSetup::kCW:
			json["culling"] = "cw";
			break;
	}
}

void fromJson(const ::nlohmann::json& json, GLSetup& value) {
	auto val = json["culling"];

	if(val == "none")
		value.setFaceCulling(GLSetup::kNone);
	else if(val == "ccv")
		value.setFaceCulling(GLSetup::kCCW);
	else if(val == "cv")
		value.setFaceCulling(GLSetup::kCW);
}

}  // namespace

IO<GLSetup> Traits<GLSetup>::io(&toJson, &fromJson);

}  // namespace possumwood
