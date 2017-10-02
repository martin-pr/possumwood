#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStyle>

#include <OpenEXR/ImathMatrix.h>

namespace {

dependency_graph::InAttr<std::string> a_src;

class Editor : public possumwood::Editor {
	public:
		Editor() : m_blockedSignals(false) {
			m_widget = new QWidget();

			QVBoxLayout* layout = new QVBoxLayout(m_widget);

			m_editor = new QPlainTextEdit();
			layout->addWidget(m_editor, 1);

			const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
			m_editor->setFont(fixedFont);

			QFontMetrics fm(fixedFont);
			m_editor->setTabStopWidth(fm.width("    "));

			QHBoxLayout* buttonsLayout = new QHBoxLayout();
			layout->addLayout(buttonsLayout, 0);

			QWidget* spacer = new QWidget();
			buttonsLayout->addWidget(spacer, 1);

			QPushButton* apply = new QPushButton();
			apply->setText("Apply (CTRL+Return)");
			apply->setIcon(apply->style()->standardIcon(QStyle::SP_DialogOkButton));
			apply->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));
			buttonsLayout->addWidget(apply);

			QObject::connect(apply, &QPushButton::pressed, [this]() {
				m_blockedSignals = true;
				values().set(a_src, m_editor->toPlainText().toStdString());
				m_blockedSignals = false;
			});

		}

		virtual ~Editor() {
		}

		virtual QWidget* widget() override {
			return m_widget;
		}

	protected:

		virtual void valueChanged(const dependency_graph::Attr& attr) override {
			if(attr == a_src && !m_blockedSignals)
				m_editor->setPlainText(values().get(a_src).c_str());
		}

	private:
		QWidget* m_widget;

		QPlainTextEdit* m_editor;

		bool m_blockedSignals;
};

dependency_graph::State checkShaderState(GLuint& shaderId) {
	dependency_graph::State state;

	GLint isCompiled = 0;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);

	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

		std::string error;
		error.resize(maxLength);
		glGetShaderInfoLog(shaderId, maxLength, &maxLength, &error[0]);

		state.addError(error);

		glDeleteShader(shaderId);
		shaderId = 0;
	}

	return state;
}

dependency_graph::State checkProgramState(GLuint& programId) {
	dependency_graph::State state;

	GLint isLinked = 0;
	glGetProgramiv(programId, GL_LINK_STATUS, &isLinked);

	if(isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

		std::string error;
		error.resize(maxLength);
		glGetProgramInfoLog(programId, maxLength, &maxLength, &error[0]);

		state.addError(error);

		glDeleteProgram(programId);
		programId = 0;
	}

	return state;
}

struct Drawable : public possumwood::Drawable {
	Drawable(dependency_graph::Values&& vals) : possumwood::Drawable(std::move(vals)) {
		m_timeChangedConnection = possumwood::App::instance().onTimeChanged([this](float t) {
				refresh();
			});
	}

	~Drawable() {
		m_timeChangedConnection.disconnect();

		if(m_posBuffer != 0) {
			glDeleteBuffers(1, &m_posBuffer);
			m_posBuffer = 0;
		}

		if(m_programId != 0 && m_vertexShaderId) {
			glDetachShader(m_programId, m_vertexShaderId);
			glDeleteShader(m_vertexShaderId);
		}

		if(m_programId != 0 && m_fragmentShaderId) {
			glDetachShader(m_programId, m_fragmentShaderId);
			glDeleteShader(m_fragmentShaderId);
		}

		if(m_programId != 0)
			glDeleteProgram(m_programId);

		if(m_vao != 0)
			glDeleteVertexArrays(1, &m_vao);
	}

	dependency_graph::State draw() {
		dependency_graph::State state;

		if(m_vertexShaderId == 0) {
			m_vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
			const GLchar* src =
				"#version 330\n"
				"\n"
				"in vec3 position;\n"
				"\n"
				"in vec3 iNearPositionVert;\n"
				"in vec3 iFarPositionVert;\n"
				"out vec3 iNearPosition;\n"
				"out vec3 iFarPosition;\n"
				"\n"
				"void main() {\n"
				"	gl_Position = vec4(position.x, position.y, position.z, 1); \n"
				"	iNearPosition = iNearPositionVert;\n"
				"	iFarPosition = iFarPositionVert;\n"
				"}";
			glShaderSource(m_vertexShaderId, 1, &src, 0);
			glCompileShader(m_vertexShaderId);

			state.append(checkShaderState(m_vertexShaderId));
		}

		if(m_fragmentShaderId == 0)
			m_fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

		const std::string& src = values().get(a_src);
		if(src != m_currentFragmentSource) {
			const char* srcPtr = src.c_str();
			glShaderSource(m_fragmentShaderId, 1, &srcPtr, 0);
			glCompileShader(m_fragmentShaderId);

			state.append(checkShaderState(m_vertexShaderId));
		}

		if(m_programId == 0)
			m_programId = glCreateProgram();

		if(src != m_currentFragmentSource && m_fragmentShaderId != 0 && m_vertexShaderId != 0) {
			glAttachShader(m_programId, m_vertexShaderId);
			glAttachShader(m_programId, m_fragmentShaderId);

			glLinkProgram(m_programId);
			state.append(checkProgramState(m_programId));

			glDetachShader(m_programId, m_vertexShaderId);
			glDetachShader(m_programId, m_fragmentShaderId);

			m_currentFragmentSource = src;

			// VAO will need updating
			if(m_vao != 0)
				glDeleteVertexArrays(1, &m_vao);
			m_vao = 0;
		}

		if(!state.errored()) {
			//////////////////////
			// setup - only once

			// first, the position buffer
			if(m_posBuffer == 0) {
				glGenBuffers(1, &m_posBuffer);

				glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer);
				static const float vertices[] = {
					-1,-1,1,
					1,-1,1,
					1,1,1,
					-1,1,1
				};
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

			// vertex array object to tie it all together
			if(m_vao == 0) {
				// make a new vertex array, and bind it - everything here is now
				//   stored inside that vertex array object
				glGenVertexArrays(1, &m_vao);
				glBindVertexArray(m_vao);

				// and tie it to the position attribute (will tie itself to CURRENT
				//   GL_ARRAY_BUFFER)
				glBindBuffer(GL_ARRAY_BUFFER, m_posBuffer);

				GLuint positionAttr = glGetAttribLocation(m_programId, "position");
				glEnableVertexAttribArray(positionAttr);
				glVertexAttribPointer(positionAttr, 3, GL_FLOAT, 0, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, 0);// unnecessary?

				glBindVertexArray(0);
			}

			//////////
			// per-frame drawing

			// bind the VAO
			glBindVertexArray(m_vao);

			// use the program
			glUseProgram(m_programId);

			// feed in the uniforms
			double modelview[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

			double projection[16];
			glGetDoublev(GL_PROJECTION_MATRIX, projection);

			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			{
				double nearPosData[12];
				gluUnProject(0, 0, 0, modelview, projection, viewport, nearPosData, nearPosData+1, nearPosData+2);
				gluUnProject(width(), 0, 0, modelview, projection, viewport, nearPosData+3, nearPosData+4, nearPosData+5);
				gluUnProject(width(), height(), 0, modelview, projection, viewport, nearPosData+6, nearPosData+7, nearPosData+8);
				gluUnProject(0, height(), 0, modelview, projection, viewport, nearPosData+9, nearPosData+10, nearPosData+11);

				if(m_nearPosBuffer == 0)
					glGenBuffers(1, &m_nearPosBuffer);
				glBindBuffer(GL_ARRAY_BUFFER, m_nearPosBuffer);

				glBufferData(GL_ARRAY_BUFFER, sizeof(nearPosData), nearPosData, GL_STATIC_DRAW);

				GLint iNearPosAttr = glGetAttribLocation(m_programId, "iNearPositionVert");
				if(iNearPosAttr >= 0) {
					glEnableVertexAttribArray(iNearPosAttr);
					glVertexAttribPointer(iNearPosAttr, 3, GL_DOUBLE, 0, 0, 0);
				}

				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

			{
				double farPosData[12];
				gluUnProject(0, 0, 1, modelview, projection, viewport, farPosData, farPosData+1, farPosData+2);
				gluUnProject(width(), 0, 1, modelview, projection, viewport, farPosData+3, farPosData+4, farPosData+5);
				gluUnProject(width(), height(), 1, modelview, projection, viewport, farPosData+6, farPosData+7, farPosData+8);
				gluUnProject(0, height(), 1, modelview, projection, viewport, farPosData+9, farPosData+10, farPosData+11);

				if(m_farPosBuffer == 0)
					glGenBuffers(1, &m_farPosBuffer);
				glBindBuffer(GL_ARRAY_BUFFER, m_farPosBuffer);

				glBufferData(GL_ARRAY_BUFFER, sizeof(farPosData), farPosData, GL_STATIC_DRAW);

				GLint iFarPosAttr = glGetAttribLocation(m_programId, "iFarPositionVert");
				if(iFarPosAttr >= 0) {
					glEnableVertexAttribArray(iFarPosAttr);
					glVertexAttribPointer(iFarPosAttr, 3, GL_DOUBLE, 0, 0, 0);

					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}
			}

			{
				GLint attr = glGetUniformLocation(m_programId, "iResolution");
				if(attr >= 0) {
					float res[2] = {(float)width(), (float)height()};
					glUniform2fv(attr, 1, res);
				}
			}

			const double near = (2.0*projection[14])/(2.0*projection[10]-2.0);
			GLint nearAttr = glGetUniformLocation(m_programId, "iNear");
			if(nearAttr >= 0)
				glUniform1d(nearAttr, near);

			GLint farAttr = glGetUniformLocation(m_programId, "iFar");
			if(farAttr >= 0) {
				const double far = ((projection[10]-1.0)*near)/(projection[10]+1.0);
				glUniform1f(farAttr, far);
			}

			// and execute draw
			glDrawArrays(GL_QUADS, 0, 4);

			// disconnect everything
			glUseProgram(0);
			glBindVertexArray(0);
		}

		return state;
	}

	boost::signals2::connection m_timeChangedConnection;

	GLuint m_programId = 0;
	GLuint m_vertexShaderId = 0;
	GLuint m_fragmentShaderId = 0;

	GLuint m_vao = 0;
	GLuint m_posBuffer = 0;
	GLuint m_nearPosBuffer = 0;
	GLuint m_farPosBuffer = 0;

	std::string m_currentFragmentSource;
};

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source", std::string(
		"#version 330\n"
		"\n"
		"uniform mat4 iProjectionMat;\n"
		"uniform mat4 iProjectionMatInv;\n"
		"uniform vec2 iResolution;\n"
		"\n"
		"layout(location=0) out vec4 color;\n"
		"\n"
		"void main() {\n"
		"	color = vec4(1,0,1,1);\n"
		"}\n"));

	meta.setDrawable<Drawable>();
	meta.setEditor<Editor>();
}

possumwood::NodeImplementation s_impl("shaders/background", init);

}
