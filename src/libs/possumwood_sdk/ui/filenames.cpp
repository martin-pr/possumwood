#include "filenames.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <QHBoxLayout>
#include <QStyle>
#include <QFileDialog>
#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QSizePolicy>

#include <possumwood_sdk/app.h>

filenames_ui::filenames_ui() {
	m_widget = new QWidget(NULL);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0,0,0,0);

	m_textEdit = new QTextEdit();

	QSizePolicy sp = m_textEdit->sizePolicy();
	sp.setVerticalPolicy(QSizePolicy::MinimumExpanding);
	m_textEdit->setSizePolicy(sp);

	m_textEdit->setLineWrapMode(QTextEdit::NoWrap);

	layout->addWidget(m_textEdit, 1);

	m_textEditConnection = QObject::connect(
		m_textEdit,
		&QTextEdit::textChanged,
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);

	m_browseButton = new QToolButton();
	m_browseButton->setIcon(m_browseButton->style()->standardIcon(QStyle::SP_DialogOpenButton));
	layout->addWidget(m_browseButton);

	m_buttonConnection = QObject::connect(
		m_browseButton,
		&QToolButton::released,
		[this]() -> void {
			// starting directory
			QStringList paths = m_textEdit->toPlainText().split('\n');
			for(auto& p : paths)
				p = possumwood::App::instance().expandPath(p.toStdString()).string().c_str();

			// run the file dialog
			paths = QFileDialog::getOpenFileNames(
				possumwood::App::instance().mainWindow(),
				"Select input files...",
				possumwood::App::instance().filename().parent_path().string().c_str(),
				boost::algorithm::join(m_value.extensions(), ";;").c_str()
			);

			for(auto& p : paths)
				p = possumwood::App::instance().shrinkPath(p.toStdString()).string().c_str();

			m_textEdit->setText(paths.join('\n'));
		}
	);
}

filenames_ui::~filenames_ui() {
	QObject::disconnect(m_textEditConnection);
	QObject::disconnect(m_buttonConnection);
}

void filenames_ui::get(possumwood::Filenames& value) const {
	value.clear();

	for(auto& s : m_textEdit->toPlainText().split('\n'))
		value.addFilename(s.toStdString());
}

void filenames_ui::set(const possumwood::Filenames& value) {
	m_value = value;

	QString content;
	for(auto& f : value.filenames())
		content = content + (f.string() + "\n").c_str();
	content = content.trimmed();

	const bool bs = m_textEdit->blockSignals(true);
	m_textEdit->setText(content);
	m_textEdit->blockSignals(bs);
}

QWidget* filenames_ui::widget() {
	return m_widget;
}

void filenames_ui::onFlagsChanged(unsigned flags) {
	assert((!(flags & kOutput)) && "Filename should never be used as an output.");

	m_browseButton->setDisabled(flags & kDisabled);
}
