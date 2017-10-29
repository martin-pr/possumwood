#include "vbo.h"

VBO::VBO() : m_widget(new QPlainTextEdit()) {
	m_widget->setReadOnly(true);
	m_widget->setWordWrapMode(QTextOption::NoWrap);

	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	m_widget->setFont(fixedFont);
}

VBO::~VBO() {
}

void VBO::get(std::shared_ptr<const possumwood::VertexData>& value) const {
	value = m_value;
}

void VBO::set(const std::shared_ptr<const possumwood::VertexData>& value) {
	if(!value) {
		m_widget->setPlainText("");
		m_widget->setMinimumHeight(0);
		m_widget->setMaximumHeight(0);
	}
	else {
		m_widget->setPlainText(value->glslDeclaration().c_str());

		QFontMetrics fm(m_widget->font());
		int h = m_widget->contentsMargins().bottom() + m_widget->contentsMargins().top() +
		        fm.lineSpacing() * value->vboCount() + m_widget->document()->documentMargin() * 2;

		m_widget->setMinimumHeight(h);
		m_widget->setMaximumHeight(h);
	}

	m_value = value;
}

QWidget* VBO::widget() {
	return m_widget;
}
