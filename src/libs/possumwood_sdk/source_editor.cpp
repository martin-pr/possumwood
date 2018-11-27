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
	m_widget = new QWidget();

	QVBoxLayout* layout = new QVBoxLayout(m_widget);

	m_editor = new QPlainTextEdit();
	layout->addWidget(m_editor, 1);
	m_editor->setWordWrapMode(QTextOption::NoWrap);

	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	m_editor->setFont(fixedFont);

	QFontMetrics fm(fixedFont);
	m_editor->setTabStopWidth(fm.width("    "));

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	layout->addLayout(buttonsLayout, 0);

	QWidget* spacer = new QWidget();
	buttonsLayout->addWidget(spacer, 1);

	QAction* applyAction = new QAction(m_widget);
	applyAction->setText("Apply (CTRL+Return)");
	applyAction->setIcon(m_widget->style()->standardIcon(QStyle::SP_DialogOkButton));
	applyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));
	m_widget->addAction(applyAction);

	QObject::connect(applyAction, &QAction::triggered, [this]() {
		m_blockedSignals = true;
		values().set(*m_src, m_editor->toPlainText().toStdString());
		m_blockedSignals = false;
	});

	QPushButton* apply = new QPushButton();
	apply->setText("Apply (CTRL+Return)");
	apply->setIcon(apply->style()->standardIcon(QStyle::SP_DialogOkButton));
	buttonsLayout->addWidget(apply);

	QObject::connect(apply, &QPushButton::pressed, [this]() {
		m_blockedSignals = true;
		values().set(*m_src, m_editor->toPlainText().toStdString());
		m_blockedSignals = false;
	});

}

SourceEditor::~SourceEditor() {
}

QWidget* SourceEditor::widget() {
	return m_widget;
}

void SourceEditor::valueChanged(const dependency_graph::Attr& attr) {
	if(attr == *m_src && !m_blockedSignals)
		m_editor->setPlainText(values().get(*m_src).c_str());
}

}
