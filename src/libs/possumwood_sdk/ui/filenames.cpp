#include "filenames.h"

#include <possumwood_sdk/app.h>

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QSizePolicy>
#include <QStyle>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

filenames_ui::filenames_ui() {
	m_widget = new QWidget(NULL);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0, 0, 0, 0);

	m_textEdit = new QTextEdit();

	QSizePolicy sp = m_textEdit->sizePolicy();
	sp.setVerticalPolicy(QSizePolicy::MinimumExpanding);
	m_textEdit->setSizePolicy(sp);

	m_textEdit->setLineWrapMode(QTextEdit::NoWrap);

	layout->addWidget(m_textEdit, 1);

	m_textEditConnection =
	    QObject::connect(m_textEdit, &QTextEdit::textChanged, [this]() -> void { callValueChangedCallbacks(); });

	m_browseButton = new QToolButton();
	m_browseButton->setIcon(m_browseButton->style()->standardIcon(QStyle::SP_DialogOpenButton));
	layout->addWidget(m_browseButton);

	m_buttonConnection = QObject::connect(m_browseButton, &QToolButton::released, [this]() -> void {
		QStringList paths = m_textEdit->toPlainText().split('\n');

		// expand all the starting paths
		for(auto& p : paths)
			p = possumwood::Filepath::fromString(p.toStdString()).toPath().string().c_str();

		// get a parent path to start at
		QString parentPath = possumwood::App::instance().filename().toPath().parent_path().string().c_str();
		if(!paths.empty())
			parentPath = boost::filesystem::path(paths[0].toStdString()).parent_path().string().c_str();

		// run the file dialog using fully expanded absolute paths
		paths = QFileDialog::getOpenFileNames(possumwood::App::instance().mainWindow(), "Select input files...",
		                                      parentPath, boost::algorithm::join(m_value.extensions(), ";;").c_str());

		// compress all the paths from the UI
		for(auto& p : paths)
			p = possumwood::Filepath::fromPath(p.toStdString()).toString().c_str();

		m_textEdit->setText(paths.join('\n'));
	});
}

filenames_ui::~filenames_ui() {
	QObject::disconnect(m_textEditConnection);
	QObject::disconnect(m_buttonConnection);
}

void filenames_ui::get(possumwood::Filenames& value) const {
	value.clear();

	for(auto& s : m_textEdit->toPlainText().split('\n'))
		value.addFilename(possumwood::Filepath::fromString(s.toStdString()));
}

void filenames_ui::set(const possumwood::Filenames& value) {
	m_value = value;

	QString content;
	for(auto& f : value.filenames())
		content = content + (f.toString() + "\n").c_str();
	content = content.trimmed();

	const bool bs = m_textEdit->blockSignals(true);
	m_textEdit->setText(content);
	m_textEdit->blockSignals(bs);
}

QWidget* filenames_ui::widget() {
	return m_widget;
}

void filenames_ui::onFlagsChanged(unsigned flags) {
	m_browseButton->setDisabled(flags & kDisabled || flags & kOutput);
	m_textEdit->setDisabled(flags & kDisabled);
	m_textEdit->setReadOnly(flags & kOutput);
}
