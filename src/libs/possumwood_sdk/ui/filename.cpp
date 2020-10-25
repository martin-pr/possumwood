#include "filename.h"

#include <actions/filepath.h>
#include <possumwood_sdk/app.h>

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QStyle>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

filename_ui::filename_ui() {
	m_widget = new QWidget(NULL);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0, 0, 0, 0);

	m_lineEdit = new QLineEdit();
	layout->addWidget(m_lineEdit, 1);

	m_lineEditConnection =
	    QObject::connect(m_lineEdit, &QLineEdit::editingFinished, [this]() -> void { callValueChangedCallbacks(); });

	m_browseButton = new QToolButton();
	m_browseButton->setIcon(m_browseButton->style()->standardIcon(QStyle::SP_DialogOpenButton));
	layout->addWidget(m_browseButton);

	m_buttonConnection = QObject::connect(m_browseButton, &QToolButton::released, [this]() -> void {
		// starting directory
		QString path = m_lineEdit->text();
		if(path.isEmpty())
			path = possumwood::App::instance().filename().toPath().parent_path().string().c_str();
		else
			// expand to a full filename, but then convert the result to a QString
			path = possumwood::Filepath::fromString(path.toStdString()).toPath().string().c_str();

		// run the file dialog
		path = QFileDialog::getOpenFileName(possumwood::App::instance().mainWindow(), "Select an input file...", path,
		                                    boost::algorithm::join(m_value.extensions(), ";;").c_str());

		if(!path.isEmpty()) {
			path = possumwood::Filepath::fromPath(path.toStdString()).toString().c_str();

			m_lineEdit->setText(path);
			m_lineEdit->editingFinished();
		}
	});
}

filename_ui::~filename_ui() {
	QObject::disconnect(m_lineEditConnection);
	QObject::disconnect(m_buttonConnection);
}

void filename_ui::get(possumwood::Filename& value) const {
	value.setFilename(m_lineEdit->text().toStdString());
}

void filename_ui::set(const possumwood::Filename& value) {
	m_value = value;

	const bool bs = m_lineEdit->blockSignals(true);
	m_lineEdit->setText(m_value.filename(false).c_str());
	m_lineEdit->blockSignals(bs);
}

QWidget* filename_ui::widget() {
	return m_widget;
}

void filename_ui::onFlagsChanged(unsigned flags) {
	m_browseButton->setDisabled(flags & kDisabled || flags & kOutput);
	m_lineEdit->setDisabled(flags & kDisabled);
	m_lineEdit->setReadOnly(flags & kOutput);
}
