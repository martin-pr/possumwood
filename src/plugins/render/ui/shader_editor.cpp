#include "shader_editor.h"

#include <dependency_graph/attr.inl>
#include <dependency_graph/values.inl>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>

namespace possumwood {

ShaderEditor::ShaderEditor(dependency_graph::InAttr<std::string>& src) : m_src(&src), m_blockedSignals(false) {
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
		values().set(*m_src, m_editor->toPlainText().toStdString());
		m_blockedSignals = false;
	});

}

ShaderEditor::~ShaderEditor() {
}

QWidget* ShaderEditor::widget() {
	return m_widget;
}

void ShaderEditor::valueChanged(const dependency_graph::Attr& attr) {
	if(attr == *m_src && !m_blockedSignals)
		m_editor->setPlainText(values().get(*m_src).c_str());
}

}
