#include "vbo.h"

VBO::VBO() : m_widget(new QPlainTextEdit()) {
	m_widget->setReadOnly(true);
	m_widget->setWordWrapMode(QTextOption::NoWrap);

	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	m_widget->setFont(fixedFont);
}

VBO::~VBO() {
}

void VBO::get(possumwood::VertexData& value) const {
	// do nothing
}

void VBO::set(const possumwood::VertexData& value) {
	m_widget->setPlainText(value.glslDeclaration().c_str());

	QFontMetrics fm(m_widget->font());
	int h = m_widget->contentsMargins().bottom() + m_widget->contentsMargins().top() +
	        fm.lineSpacing() * value.vboCount() + m_widget->document()->documentMargin() * 2;

	m_widget->setMinimumHeight(h);
	m_widget->setMaximumHeight(h);

	m_value = value.glslDeclaration();
}

QWidget* VBO::widget() {
	return m_widget;
}
