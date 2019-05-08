#include "source_editor.h"

#include <dependency_graph/attr.inl>
#include <dependency_graph/values.inl>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QAction>

namespace possumwood {

SourceEditor::SourceEditor(dependency_graph::InAttr<std::string>& src) : m_src(&src), m_blockedSignals(false) {
	QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(this->layout());
	assert(layout != nullptr);

	m_editor = new QPlainTextEdit();
	layout->addWidget(m_editor, 1);
	m_editor->setWordWrapMode(QTextOption::NoWrap);

	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	m_editor->setFont(fixedFont);

	QFontMetrics fm(fixedFont);
	m_editor->setTabStopWidth(fm.width("    "));

	m_buttonsLayout = new QHBoxLayout();
	layout->addLayout(m_buttonsLayout, 0);

	QWidget* spacer = new QWidget();
	m_buttonsLayout->addWidget(spacer, 1);

	QAction* applyAction = new QAction(this);
	applyAction->setText("Apply (CTRL+Return)");
	applyAction->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
	applyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));
	addAction(applyAction);

	QObject::connect(applyAction, &QAction::triggered, [this]() {
		m_blockedSignals = true;
		values().set(*m_src, m_editor->toPlainText().toStdString());
		m_blockedSignals = false;
	});

	QPushButton* apply = new QPushButton();
	apply->setText("Apply (CTRL+Return)");
	apply->setIcon(apply->style()->standardIcon(QStyle::SP_DialogOkButton));
	m_buttonsLayout->addWidget(apply);

	QObject::connect(apply, &QPushButton::pressed, [this]() {
		m_blockedSignals = true;
		values().set(*m_src, m_editor->toPlainText().toStdString());
		m_blockedSignals = false;
	});

}

SourceEditor::~SourceEditor() {
}

void SourceEditor::valueChanged(const dependency_graph::Attr& attr) {
	if(attr == *m_src && !m_blockedSignals)
		m_editor->setPlainText(values().get(*m_src).c_str());
}

QHBoxLayout* SourceEditor::buttonsLayout() const {
	return m_buttonsLayout;
}

QPlainTextEdit* SourceEditor::editorWidget() const {
	return m_editor;
}

}
